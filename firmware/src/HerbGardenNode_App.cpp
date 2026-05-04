/**
 * @file HerbGardenNode_App.cpp
 * @author Johnny - IoT System Architect
 * @version 2.2.0-MULTICORE
 * @brief Dual-Core Industrial IoT Node Implementation.
 * @standard US Technical Standards - Embedded Architecture.
 *
 * Architectural Allocation:
 * - CORE 0 (Primary): Network Orchestration, Hardware RNG, and Secure UDP Dispatch.
 * - CORE 1 (Secondary): Deterministic Telemetry Acquisition and Cryptographic Engine (ChaCha20-Poly1305).
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pico/mutex.h>
#include <RNG.h>

extern "C"
{
#include <pico/cyw43_arch.h>
}

#include "AppConfig.h"
#include "Secrets.h"
#include "models.h"
#include "controllers/HydrationController.h"
#include "controllers/ClimateController.h"
#include "controllers/NetworkController.h"
#include "controllers/SecurityController.h"

// --- HARDWARE ABSTRACTION LAYER (HAL) INSTANCES ---
HydrationController hydrationCtrl(PIN_SOIL_SENSOR, SOIL_AIR_DRY, SOIL_WATER_WET);
ClimateController climateCtrl(PIN_DHT_SENSOR, DHT11);
NetworkController networkCtrl;
SecurityController securityCtrl(CHACHA_KEY);
WiFiUDP udpClient;

// --- INTER-CORE SYNCHRONIZATION (SHARED MEMORY) ---
// Mutex to prevent race conditions during cross-core payload transfer
mutex_t packet_mutex;
std::string shared_secure_packet = "";
volatile bool is_packet_ready = false;
String active_mac_identity;

// ==============================================================================
// CORE 0: NETWORK ORCHESTRATOR & INGRESS DISPATCHER
// ==============================================================================

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println(F("\n[CORE 0] System Boot Sequence Initiated."));

    // 1. Initialize Cross-Core Mutex
    mutex_init(&packet_mutex);

    // 2. Hardware Entropy Stirring (Cryptographic Prerequisite)
    RNG.begin("HerbGardenV2");
    uint32_t entropy = rp2040.hwrand32();
    RNG.stir((const uint8_t *)&entropy, sizeof(entropy));

    // 3. Identity Configuration & MAC Spoofing (Stealth Mode)
    networkCtrl.begin();
    active_mac_identity = networkCtrl.getPhysicalMac();

    uint8_t stealth_mac[6];
    for (int i = 0; i < 6; i++)
    {
        stealth_mac[i] = (uint8_t)rp2040.hwrand32();
    }
    stealth_mac[0] = (stealth_mac[0] & 0xFC) | 0x02; // Set LAA bit
    networkCtrl.applyMacSpoofing(stealth_mac);

    // 4. WLAN Initialization
    Serial.print(F("[CORE 0] Negotiating WLAN Connection"));
    WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(F("."));
    }
    Serial.printf("\n[CORE 0] Network Link Established. IP Allocation: %s\n", WiFi.localIP().toString().c_str());
}

void loop()
{
    // 1. Connectivity Watchdog
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("[CORE 0] WARN: Network linkage lost. Initiating recovery..."));
        WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
        delay(5000); // Backoff delay
        return;
    }

    // 2. Inter-Core Payload Dispatch Queue
    if (is_packet_ready)
    {
        std::string dispatch_buffer;

        // Acquire Mutex: Safely extract the cryptographic payload
        mutex_enter_blocking(&packet_mutex);
        dispatch_buffer = shared_secure_packet;
        is_packet_ready = false; // Acknowledge receipt, free Core 1 for next cycle
        mutex_exit(&packet_mutex);

        // 3. UDP Datagram Transmission
        Serial.print(F("[CORE 0] Dispatching AEAD Datagram... "));
        udpClient.beginPacket(SERVER_HOSTNAME, SERVER_PORT);
        udpClient.write((const uint8_t *)dispatch_buffer.data(), dispatch_buffer.length());

        if (udpClient.endPacket())
        {
            Serial.println(F("Success."));
        }
        else
        {
            Serial.println(F("Failed (Packet Dropped)."));
        }
    }

    // Yield to prevent RTOS starvation and bus locking
    delay(10);
}

// ==============================================================================
// CORE 1: SENSOR ACQUISITION & CRYPTOGRAPHIC ENGINE
// ==============================================================================

unsigned long previous_tx_millis = 0;
const unsigned long TX_DUTY_CYCLE_MS = 5000; // 5-second telemetry interval

void setup1()
{
    // Wait for Core 0 to initialize the primary Serial interface
    delay(3000);
    Serial.println(F("[CORE 1] Telemetry & Cryptography Engine Online."));

    // Initialize Peripheral Hardware
    hydrationCtrl.begin();
    climateCtrl.begin();
}

void loop1()
{
    unsigned long current_millis = millis();

    // Deterministic execution loop independent of WLAN state
    if (current_millis - previous_tx_millis >= TX_DUTY_CYCLE_MS)
    {
        previous_tx_millis = current_millis;

        ClimateModel climate_data;
        SoilHydrationModel hydration_data;

        // 1. Synchronous Hardware Sampling
        climateCtrl.update(climate_data);
        hydrationCtrl.update(hydration_data);

        // 2. Verification & Cryptographic Processing
        // Proceed only if sensor data is valid and the dispatch queue is clear
        if (climate_data.is_valid && !is_packet_ready)
        {

            // Construct the plaintext telemetry payload
            String raw_payload = "ID:" + active_mac_identity +
                                 ",T:" + String(climate_data.temperature_celsius) +
                                 ",H:" + String(climate_data.humidity_percentage) +
                                 ",S:" + String(hydration_data.saturation_percentage);

            // Execute ChaCha20-Poly1305 AEAD Encryption (Computationally intensive)
            std::string secure_packet = securityCtrl.encryptPayload(raw_payload.c_str());

            // 3. Inter-Core Synchronization
            // Lock the mutex to push the generated packet to Core 0
            mutex_enter_blocking(&packet_mutex);
            shared_secure_packet = secure_packet;
            is_packet_ready = true; // Flag for Core 0 to dispatch
            mutex_exit(&packet_mutex);

            Serial.println(F("[CORE 1] Telemetry encrypted and pushed to dispatch queue."));
        }
        else if (!climate_data.is_valid)
        {
            Serial.println(F("[CORE 1] ERROR: Sensor data integrity check failed."));
        }
    }

    // Prevent Core 1 from locking the system bus
    delay(10);
}