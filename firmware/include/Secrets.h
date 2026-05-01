#ifndef SECRETS_H
#define SECRETS_H

#include <Arduino.h>

// WiFi Credentials
struct WiFiNetwork
{
    const char *ssid;
    const char *pass;
};

// List of WiFi networks to connect to (SSID and Password)
const WiFiNetwork MY_NETWORKS[] = {
    {"HungPhat24G", "44448888"},              // Home WiFi
    //{"Greenwich-Guest 2.4G", "greenwichvn@123"}, // Office WiFi
    //{"Redmi Note 13 của Phát", "44448888"}, // Mobile Hotspot
};

const int NETWORK_COUNT = sizeof(MY_NETWORKS) / sizeof(MY_NETWORKS[0]); // Number of WiFi networks in the list

// ChaCha20-Poly1305 Security Keys (32-byte key)
const uint8_t CHACHA_KEY[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

#endif