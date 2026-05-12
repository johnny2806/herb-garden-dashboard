/**
 * @file HerbGardenNode_App.cpp
 * @author Johnny - IoT System Architect
 * @version 2.5.0-STABLE
 * @brief Dual-Core Industrial IoT Node for Herb Garden Monitoring.
 * @standard US Technical Standards - Embedded Systems Architecture.
 *
 * Architectural Design:
 * - CORE 0 (Primary): WLAN Management, ADC Data Acquisition (CYW43 Conflict Mitigation),
 * Hardware RNG, and Secure UDP Ingress Dispatch.
 * - CORE 1 (Secondary): Environmental Telemetry (1-Wire), System Health Monitoring,
 * and Cryptographic Engine (ChaCha20-Poly1305).
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
#include "controllers/SystemHealthController.h"

// --- HARDWARE ABSTRACTION LAYER (HAL) INSTANCES ---
HydrationController hydrationCtrl(PIN_SOIL_SENSOR, SOIL_AIR_DRY, SOIL_WATER_WET);
ClimateController climateCtrl(PIN_DHT_SENSOR, DHT11);
NetworkController networkCtrl;
SecurityController securityCtrl(CHACHA_KEY);
WiFiUDP udpClient;
SystemHealthController healthCtrl;

// --- INTER-CORE SYNCHRONIZATION (SHARED MEMORY MUTEXES) ---
// Mutex 1: Protects encrypted datagram payload during cross-core handover
mutex_t packet_mutex;
std::string shared_secure_packet = "";
volatile bool is_packet_ready = false;

// Mutex 2: Protects raw ADC sensor data ferried from Core 0 to Core 1
mutex_t sensor_mutex;
SoilHydrationModel shared_hydration_data;

String active_mac_identity;

// ==============================================================================
// CORE 0: NETWORK ORCHESTRATOR, ADC ACQUISITION & INGRESS DISPATCHER
// ==============================================================================

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println(F("\n[CORE 0] SYS: Boot sequence initiated."));

    // 0. Hardware Subsystem Configuration
    // Standard 10-bit resolution (0-1023) for improved stability with WiFi driver
    analogReadResolution(10);

    // 1. Initialize Cross-Core Mutexes
    mutex_init(&packet_mutex);
    mutex_init(&sensor_mutex);

    // 2. Hardware Entropy Stirring (Cryptographic Prerequisite)
    RNG.begin("HerbGardenV2");
    uint32_t entropy = rp2040.hwrand32();
    RNG.stir((const uint8_t *)&entropy, sizeof(entropy));

    // 3. Identity Configuration & Network Initialization
    networkCtrl.begin();
    active_mac_identity = networkCtrl.getPhysicalMac();

    // 4. WLAN Subsystem Negotiation
    Serial.print(F("[CORE 0] NET: Negotiating WLAN link layer"));
    WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(F("."));
    }
    Serial.printf("\n[CORE 0] NET: Link established. IP Allocation: %s\n", WiFi.localIP().toString().c_str());
}

void loop()
{
    // 1. Connectivity Watchdog & Keep-Alive
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("[CORE 0] WARN: Network linkage degraded. Initiating recovery..."));
        WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
        delay(5000);
        return;
    }

    // 2. ADC Hardware Acquisition (Executing on Core 0 to prevent CYW43 Driver conflicts)
    static unsigned long previous_adc_millis = 0;
    const unsigned long ADC_SAMPLING_RATE_MS = 2000; // Synchronized with Core 1 cycle

    if (millis() - previous_adc_millis >= ADC_SAMPLING_RATE_MS)
    {
        previous_adc_millis = millis();

        SoilHydrationModel local_hydration;
        // Perform direct analog sampling
        hydrationCtrl.update(local_hydration);

        // Safe-handover of analog data to Shared Memory for Core 1 consumption
        mutex_enter_blocking(&sensor_mutex);
        shared_hydration_data = local_hydration;
        mutex_exit(&sensor_mutex);
    }

    // 3. Inter-Core Payload Dispatch Queue
    if (is_packet_ready)
    {
        std::string dispatch_buffer;

        // Atomic extraction of ciphertext from Core 1
        mutex_enter_blocking(&packet_mutex);
        dispatch_buffer = shared_secure_packet;
        is_packet_ready = false;
        mutex_exit(&packet_mutex);

        // UDP Ingress Transmission
        udpClient.beginPacket(SERVER_HOSTNAME, SERVER_PORT);
        udpClient.write((const uint8_t *)dispatch_buffer.data(), dispatch_buffer.length());

        if (udpClient.endPacket())
        {
            Serial.println(F("[CORE 0] INGRESS: Secure datagram successfully dispatched."));
        }
        else
        {
            Serial.println(F("[CORE 0] ERROR: Ingress dispatch failed (L4 Packet Drop)."));
        }
    }

    // Yield to background system tasks
    delay(10);
}

// ==============================================================================
// CORE 1: SENSOR ACQUISITION & CRYPTOGRAPHIC ENGINE
// ==============================================================================

unsigned long previous_tx_millis = 0;
const unsigned long TX_DUTY_CYCLE_MS = 2000; // 2-second deterministic reporting interval

void setup1()
{
    // Await Primary Core HAL stabilization
    delay(3000);
    Serial.println(F("[CORE 1] SYS: Telemetry and Cryptographic Engine online."));

    // Initialize Digital Peripherals
    climateCtrl.begin();
    healthCtrl.begin();
}

void loop1()
{
    unsigned long current_millis = millis();

    // Deterministic Telemetry Loop
    if (current_millis - previous_tx_millis >= TX_DUTY_CYCLE_MS)
    {
        previous_tx_millis = current_millis;

        ClimateModel climate_data;
        SoilHydrationModel hydration_data;
        SystemHealthModel health_data;
        NetworkModel network_data;

        // 1. Synchronous Digital Sensor Sampling
        climateCtrl.update(climate_data);
        healthCtrl.update(health_data);
        networkCtrl.update(network_data);

        // 2. Cross-Core Data Retrieval (Acquire ADC data from Core 0)
        mutex_enter_blocking(&sensor_mutex);
        hydration_data = shared_hydration_data;
        mutex_exit(&sensor_mutex);

        // 3. Data Integrity Verification & Packaging
        if (!is_packet_ready)
        {
            // Debug telemetry matrix for serial diagnostics
            Serial.printf("[CORE 1] DIAG: T:%.1fC | H:%.1f%% | SoilRaw:%d (%.1f%%) | Uptime:%lus\n",
                          climate_data.temperature_celsius,
                          climate_data.humidity_percentage,
                          hydration_data.raw_value,
                          hydration_data.saturation_percentage,
                          health_data.uptime_ms / 1000);

            // Construct plaintext JSON-compatible telemetry string
            String raw_payload = "ID:" + active_mac_identity +
                                 ",T:" + String(climate_data.temperature_celsius) +
                                 ",H:" + String(climate_data.humidity_percentage) +
                                 ",S:" + String(hydration_data.saturation_percentage) +
                                 ",SR:" + String(hydration_data.raw_value) +
                                 ",CT:" + String(health_data.core_temp, 1) +
                                 ",MEM:" + String(health_data.free_heap_bytes) +
                                 ",RSSI:" + String(network_data.signal_strength_rssi) +
                                 ",UP:" + String(health_data.uptime_ms / 1000);

            // Execute ChaCha20-Poly1305 AEAD Encryption
            std::string secure_packet = securityCtrl.encryptPayload(raw_payload.c_str());

            // 4. Critical Section Handover
            mutex_enter_blocking(&packet_mutex);
            shared_secure_packet = secure_packet;
            is_packet_ready = true; // Signal Core 0 for dispatch
            mutex_exit(&packet_mutex);
        }
        else
        {
            Serial.println(F("[CORE 1] WARN: Dispatch queue saturated. Dropping current frame."));
        }
    }

    // Prevent system bus contention
    delay(10);
}