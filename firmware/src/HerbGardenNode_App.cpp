/**
 * Project: SECURE HERB GARDEN IOT NODE (US Technical Standard)
 * Feature: Binary Counter Nonce, ChaCha20 Stream Cipher, Randomized MAC
 */

#include <RNG.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "AppConfig.h"
#include "Secrets.h"
#include "models.h"
#include "controllers/HydrationController.h"
#include "controllers/ClimateController.h"
#include "controllers/NetworkController.h"
#include "controllers/SecurityController.h"

// Hardware RNG and Network Access Definitions
extern "C"
{
#include <pico/cyw43_arch.h>
}

// --- GLOBAL INSTANCES ---
HydrationController hydrationCtrl(PIN_SOIL_SENSOR, SOIL_AIR_DRY, SOIL_WATER_WET);
ClimateController climateCtrl(PIN_DHT_SENSOR, DHT11);
NetworkController networkCtrl;
SecurityController securityCtrl(CHACHA_KEY);
WiFiUDP udpClient;

// --- DATA MODELS ---
ClimateModel climateData;
SoilHydrationModel hydrationData;
String trueIdentityMAC;

unsigned long previousMillis = 0;
const long TX_INTERVAL = 5000; // Duty Cycle: 5-second telemetry interval

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println(F("\n[SYSTEM] SECURE NODE INITIALIZING..."));

    // 1. Hardware Entropy Stirring
    RNG.begin("HerbGardenV2");
    uint32_t entropy = rp2040.hwrand32();
    RNG.stir((const uint8_t *)&entropy, sizeof(entropy));

    // 2. Identity & Stealth Masking
    networkCtrl.begin();
    trueIdentityMAC = networkCtrl.getPhysicalMac();

    uint8_t stealthMac[6];
    // Generate locally administered MAC address
    for (int i = 0; i < 6; i++)
        stealthMac[i] = (uint8_t)rp2040.hwrand32();
    stealthMac[0] = (stealthMac[0] & 0xFC) | 0x02;
    networkCtrl.applyMacSpoofing(stealthMac);

    // 3. Network Ingress
    WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[NET] Link Established. IP: %s\n", WiFi.localIP().toString().c_str());

    hydrationCtrl.begin();
    climateCtrl.begin();
}

void loop()
{
    static unsigned long heartbeat = 0;
    if (millis() - heartbeat > 2000)
    {
        Serial.println(F("[System] Heartbeat: Looping..."));
        heartbeat = millis();
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= TX_INTERVAL)
    {
        previousMillis = currentMillis;

        climateCtrl.update(climateData);
        hydrationCtrl.update(hydrationData);

        if (climateData.is_valid && WiFi.status() == WL_CONNECTED)
        {
            // 1. Payload Serialization (CSV Key-Value Pairs)
            String rawPayload = "ID:" + trueIdentityMAC +
                                ",T:" + String(climateData.temperature_celsius) +
                                ",H:" + String(climateData.humidity_percentage) +
                                ",S:" + String(hydrationData.saturation_percentage);

            // 2. Cryptographic Transformation
            // Uses the binary counter-based nonce architecture
            std::string securePacket = securityCtrl.encryptPayload(rawPayload.c_str());

            // 3. UDP Dispatch (O(1) complexity)
            // Transmission of binary datagram [Counter (4B)][Ciphertext]
            Serial.print(F("[UDP] Ingesting secure datagram... "));
            udpClient.beginPacket(SERVER_HOSTNAME, SERVER_PORT);
            udpClient.write((const uint8_t *)securePacket.data(), securePacket.length());

            if (udpClient.endPacket())
            {
                Serial.println(F("Success."));
            }
            else
            {
                Serial.println(F("Packet Drop Detected."));
            }
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
        delay(5000);
    }
}