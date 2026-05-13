/*
 * @file HerbGardenNode_App.cpp
 * @author Johnny (Nguyen Tan Phat) - IoT System Architect
 * @version 3.2.0-NATIVE-BYPASS-FINAL
 * @brief Dual-Core Industrial IoT Node for Herb Garden Monitoring.
 * @standard US Technical Standards - Embedded Systems Architecture.
 *
 * Architectural Design (ADC Shielding Implementation):
 * - CORE 0 (Primary): WLAN Management, Native SDK ADC Data Acquisition,
 * Hardware RNG, Secure UDP Ingress Dispatch, and HTTP Command Polling.
 * - CORE 1 (Secondary): Digital Telemetry (1-Wire), System Health Monitoring,
 * Actuator Override Logic, and Cryptographic Engine (ChaCha20-Poly1305).
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pico/mutex.h>
#include <RNG.h>

// Core network libraries for command polling
#include <HTTPClient.h>
#include <ArduinoJson.h>

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
#include "controllers/ActuatorController.h"

// --- HARDWARE ABSTRACTION LAYER (HAL) INSTANCES ---
HydrationController hydrationCtrl(PIN_SOIL_SENSOR, SOIL_AIR_DRY, SOIL_WATER_WET);
ClimateController climateCtrl(PIN_DHT_SENSOR, DHT11);
NetworkController networkCtrl;
SecurityController securityCtrl(CHACHA_KEY);
WiFiUDP udpClient;
SystemHealthController healthCtrl;
ActuatorController actuatorCtrl(PIN_PUMP_RELAY);

// --- INTER-CORE SYNCHRONIZATION (SHARED MEMORY MUTEXES) ---
// Mutex 1: Protects encrypted datagram payload during cross-core handover
mutex_t packet_mutex;
std::string shared_secure_packet = "";
volatile bool is_packet_ready = false;

// Mutex 2: Protects raw ADC sensor data ferried from Core 0 to Core 1
mutex_t sensor_mutex;
SoilHydrationModel shared_hydration_data;

// Shared Command Flag: -1 (Auto Mode), 0 (Force Disengage), 1 (Force Engage)
volatile int shared_pump_cmd = -1;
String active_mac_identity;

// ==============================================================================
// CORE 0: NETWORK ORCHESTRATOR, NATIVE ADC ACQUISITION & INGRESS DISPATCHER
// ==============================================================================

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println(F("\n[CORE 0] SYS: Boot sequence initiated."));

    // 1. Initialize Hardware Subsystems (Native Pico SDK Bypass)
    hydrationCtrl.begin();

    // 2. Initialize Cross-Core Shared Memory Mutexes
    mutex_init(&packet_mutex);
    mutex_init(&sensor_mutex);

    // 3. Hardware Entropy Stirring (Cryptographic Prerequisite)
    RNG.begin("HerbGardenV3");
    uint32_t entropy = rp2040.hwrand32();
    RNG.stir((const uint8_t *)&entropy, sizeof(entropy));

    // 4. Identity Configuration & Network Initialization
    networkCtrl.begin();
    active_mac_identity = networkCtrl.getPhysicalMac();

    // 5. WLAN Subsystem Negotiation
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
        Serial.println(F("[CORE 0] WARN: Network linkage degraded. Initiating recovery protocols..."));
        WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
        delay(5000);
        return;
    }

    // 2. Analog Hardware Acquisition (Executing safely on Core 0)
    static unsigned long previous_adc_millis = 0;
    const unsigned long ADC_SAMPLING_RATE_MS = 1000;

    if (millis() - previous_adc_millis >= ADC_SAMPLING_RATE_MS)
    {
        previous_adc_millis = millis();

        SoilHydrationModel local_hydration;
        hydrationCtrl.update(local_hydration);

        // Atomic handover of analog data to Shared Memory for Core 1 consumption
        mutex_enter_blocking(&sensor_mutex);
        shared_hydration_data = local_hydration;
        mutex_exit(&sensor_mutex);
    }

    // 3. Inter-Core Payload Dispatch Queue Management
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
            Serial.println(F("[CORE 0] INGRESS: Secure AEAD datagram successfully dispatched."));
        }
        else
        {
            Serial.println(F("[CORE 0] ERROR: Ingress dispatch failed (L4 Packet Drop)."));
        }
    }

    // 4. Actuator Command Polling (REST API Integration)
    static unsigned long last_cmd_poll = 0;
    const unsigned long COMMAND_POLL_RATE_MS = 1000;

    if (millis() - last_cmd_poll >= COMMAND_POLL_RATE_MS)
    {
        last_cmd_poll = millis();

        WiFiClient client;
        HTTPClient http;

        // Explicit URL construction
        String cmd_url = "http://" + String(SERVER_HOSTNAME) + ":8000/api/v1/telemetry/latest";

        http.setTimeout(1000);
        http.setReuse(false); // Mitigate TCP socket exhaustion (Anti-Leak)

        if (http.begin(client, cmd_url))
        {
            http.addHeader("Connection", "close"); // Force server-side closure

            int httpCode = http.GET();
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = http.getString();
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (!error)
                {
                    if (doc["command"].is<bool>())
                    {
                        shared_pump_cmd = doc["command"].as<bool>() ? 1 : 0;
                        Serial.printf("[CORE 0] COMMAND OVERRIDE: %s\n", shared_pump_cmd == 1 ? "FORCE_ENGAGE" : "FORCE_DISENGAGE");
                    }
                    else if (doc["command"].isNull())
                    {
                        shared_pump_cmd = -1;
                        Serial.println(F("[CORE 0] COMMAND OVERRIDE: REVERT_TO_AUTO"));
                    }
                }
            }
            else
            {
                Serial.printf("[CORE 0] ERROR: Telemetry server responded with HTTP %d\n", httpCode);
            }
            http.end(); // Deallocate HTTP resources
        }

        // Critical safeguard: forcefully terminate the socket to prevent heap fragmentation
        client.stop();
    }

    // Yield RTOS thread to prevent background task starvation
    delay(10);
}

// ==============================================================================
// CORE 1: DIGITAL SENSOR ACQUISITION & CRYPTOGRAPHIC ENGINE
// ==============================================================================

unsigned long previous_tx_millis = 0;
const unsigned long TX_DUTY_CYCLE_MS = 1000; // Deterministic reporting interval (1.0s)

void setup1()
{
    // Await Primary Core HAL stabilization
    delay(3000);
    Serial.println(F("[CORE 1] SYS: Telemetry and Cryptographic Engine online."));

    // Initialize Digital Peripheral Subsystems
    climateCtrl.begin();
    healthCtrl.begin();
    actuatorCtrl.begin();
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
        ActuatorModel actuator_data;

        // 1. Synchronous Digital Sensor Sampling
        climateCtrl.update(climate_data);
        healthCtrl.update(health_data);
        networkCtrl.update(network_data);

        // 2. Cross-Core Data Retrieval (Acquire Native ADC data ferried from Core 0)
        mutex_enter_blocking(&sensor_mutex);
        hydration_data = shared_hydration_data;
        mutex_exit(&sensor_mutex);

        // ==============================================================================
        // --- AUTOMATION & MANUAL OVERRIDE ENGINE ---
        // ==============================================================================
        int current_cmd = shared_pump_cmd; // Read the latest command from shared memory

        if (current_cmd == -1)
        {
            // AUTOMATIC CONTROL REGIME
            const float MOISTURE_THRESHOLD = 60.0f;
            if (hydration_data.saturation_percentage < MOISTURE_THRESHOLD)
            {
                actuatorCtrl.activate();
            }
            else
            {
                actuatorCtrl.deactivate();
            }
        }
        else
        {
            // MANUAL OVERRIDE REGIME
            if (current_cmd == 1)
            {
                actuatorCtrl.activate();
            }
            else
            {
                actuatorCtrl.deactivate();
            }
        }
        // ==============================================================================

        // 3. Update Actuator State AFTER automation logic to ensure accurate payload
        actuatorCtrl.update(actuator_data);

        // 4. Data Integrity Verification & Cryptographic Packaging
        if (!is_packet_ready)
        {
            // Real-time telemetry matrix for serial diagnostics (Including Actuator State)
            Serial.printf("[CORE 1] DIAG: T:%.1fC | H:%.1f%% | SoilRaw:%d (%.1f%%) | Uptime:%lus | PUMP:%s\n",
                          climate_data.temperature_celsius,
                          climate_data.humidity_percentage,
                          hydration_data.raw_value,
                          hydration_data.saturation_percentage,
                          health_data.uptime_ms / 1000,
                          actuator_data.is_active ? "ENGAGED" : "STANDBY");

            // Construct plaintext JSON-compatible telemetry payload
            String raw_payload = "ID:" + active_mac_identity +
                                 ",T:" + String(climate_data.temperature_celsius) +
                                 ",H:" + String(climate_data.humidity_percentage) +
                                 ",S:" + String(hydration_data.saturation_percentage) +
                                 ",SR:" + String(hydration_data.raw_value) +
                                 ",CT:" + String(health_data.core_temp, 1) +
                                 ",MEM:" + String(health_data.free_heap_bytes) +
                                 ",RSSI:" + String(network_data.signal_strength_rssi) +
                                 ",UP:" + String(health_data.uptime_ms / 1000) +
                                 ",PMP:" + String(actuator_data.is_active ? 1 : 0);

            // Execute ChaCha20-Poly1305 AEAD Encryption (Computationally intensive task assigned to Core 1)
            std::string secure_packet = securityCtrl.encryptPayload(raw_payload.c_str());

            // 5. Critical Section Handover
            mutex_enter_blocking(&packet_mutex);
            shared_secure_packet = secure_packet;
            is_packet_ready = true; // Signal Core 0 to dispatch the packet
            mutex_exit(&packet_mutex);
        }
        else
        {
            Serial.println(F("[CORE 1] WARN: Dispatch queue saturated. Dropping current frame."));
        }
    }

    // Yield system bus to prevent contention
    delay(10);
}