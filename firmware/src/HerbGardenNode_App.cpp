/**
 * Project: SECURE HERB GARDEN IOT NODE
 * Feature: Randomized MAC Spoofing, ChaCha20 Encryption, UDP Telemetry
 * Language: C++ (Arduino/Pico SDK)
 * Standards: US English, Clean Code, O(n) Encryption Complexity
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

// Hardware RNG and Network Access
extern "C"
{
#include <pico/cyw43_arch.h>
}

// --- GLOBAL INSTANCES ---
HydrationController hydrationCtrl(PIN_SOIL_SENSOR, SOIL_AIR_DRY, SOIL_WATER_WET);
ClimateController climateCtrl(PIN_DHT_SENSOR, DHT11);
NetworkController networkCtrl;
SecurityController securityCtrl(CHACHA_KEY);
WiFiUDP udpClient; // Handles O(1) datagram transmission

// --- DATA MODELS ---
ClimateModel climateData;
SoilHydrationModel hydrationData;
String trueIdentityMAC; // The permanent ID used for Backend Auth

// Add variable for Non-blocking Delay and Sequence Number
unsigned long previousMillis = 0;
const long TX_INTERVAL = 5000; // Duty Cycle: 5000ms (5 seconds)
uint32_t messageSequence = 0;  // Counter for Replay Attack Protection

/**
 * Helper: Converts byte array to Uppercase Hex String.
 * Complexity: O(n) where n is buffer length.
 */
String toHexString(uint8_t *data, size_t len)
{
    String output = "";
    for (size_t i = 0; i < len; i++)
    {
        if (data[i] < 0x10)
            output += "0";
        output += String(data[i], HEX);
    }
    output.toUpperCase();
    return output;
}

/**
 * Generates a random locally administered MAC address.
 * Standard: First byte ends in '2' (e.g., 02:XX:XX...) for compatibility.
 */
void generateStealthMAC(uint8_t *mac)
{
    for (int i = 0; i < 6; i++)
    {
        mac[i] = (uint8_t)rp2040.hwrand32();
    }
    // Set 'Locally Administered' bit (0x02) and Clear 'Multicast' bit
    mac[0] = (mac[0] & 0xFC) | 0x02;
}

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println(F("\n==========================================="));
    Serial.println(F("   SECURE STEALTH NODE: INITIALIZING..."));
    Serial.println(F("==========================================="));

    // 1. Hardware Cryptography Setup
    RNG.begin("HerbGardenV2");
    uint32_t entropy = rp2040.hwrand32();
    RNG.stir((const uint8_t *)&entropy, sizeof(entropy));

    // 2. Identity & Stealth Masking
    networkCtrl.begin();
    trueIdentityMAC = networkCtrl.getPhysicalMac(); // Keep real MAC for secure payload
    Serial.println("[Identity] True Hardware MAC: " + trueIdentityMAC);

    uint8_t stealthMac[6];
    generateStealthMAC(stealthMac);
    networkCtrl.applyMacSpoofing(stealthMac);
    Serial.println("[Stealth] Mask Applied:      " + networkCtrl.getActiveMac());

    // 3. Network Ingress via Mobile Hotspot
    Serial.printf("[Network] Connecting to SSID: %s\n", MY_NETWORKS[0].ssid);
    WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[Network] Online! Node IP: %s\n", WiFi.localIP().toString().c_str());

    // 4. Sensor Initialization
    hydrationCtrl.begin();
    climateCtrl.begin();
    Serial.println(F("[System] Telemetry sensors active."));
}

void loop()
{
    // 1. Data Acquisition
    climateCtrl.update(climateData);
    hydrationCtrl.update(hydrationData);

    unsigned long currentMillis = millis();

    // Use Non-blocking Delay for Telemetry Transmission
    if (currentMillis - previousMillis >= TX_INTERVAL)
    {
        previousMillis = currentMillis;

        // 1. Data Acquisition
        climateCtrl.update(climateData);
        hydrationCtrl.update(hydrationData);

        // 2. Process Telemetry if WiFi is stable
        if (climateData.is_valid && WiFi.status() == WL_CONNECTED)
        {

            messageSequence++; // Increase sequence number for each message to prevent replay attacks

            // Construct Raw Secure Payload (Đã thêm SEQ)
            String rawPayload = "SEQ:" + String(messageSequence) +
                                ",ID:" + trueIdentityMAC +
                                ",T:" + String(climateData.temperature_celsius) +
                                ",H:" + String(climateData.humidity_percentage) +
                                ",S:" + String(hydrationData.saturation_percentage);

            // 3. Encryption Layer: ChaCha20-Poly1305 O(n)
            uint8_t ciphertext[128], tag[16], iv[12];
            RNG.rand(iv, 12);
            securityCtrl.encrypt(rawPayload, ciphertext, tag, iv);

            // 4. Protocol Formatting: "IV:<hex>|CIPHER:<hex>"
            String ivHex = toHexString(iv, 12);
            String cipherHex = toHexString(ciphertext, rawPayload.length());
            String finalDatagram = "IV:" + ivHex + "|CIPHER:" + cipherHex;

            // 5. UDP Dispatch O(1) complexity
            Serial.print(F("[UDP] Ingesting telemetry to Gateway... "));
            udpClient.beginPacket(SERVER_HOSTNAME, SERVER_PORT);
            udpClient.write((const uint8_t *)finalDatagram.c_str(), finalDatagram.length());

            if (udpClient.endPacket())
            {
                Serial.println(F("Success."));
            }
            else
            {
                Serial.println(F("Transmission Error."));
            }
        }
    }

    // Handle WiFi Reconnection if Lost
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("[Alert] Connection lost. Attempting Re-ingress..."));
        WiFi.begin(MY_NETWORKS[0].ssid, MY_NETWORKS[0].pass);
        delay(5000); // Prevent spamming connection attempts, adjust as needed for faster recovery
    }

    // Duty Cycle: 1 seconds for Real-time Dashboard Feel
    delay(2000);
}