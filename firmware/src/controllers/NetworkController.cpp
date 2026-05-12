#include "controllers/NetworkController.h"
#include <WiFi.h>

extern "C"
{
#include <pico/cyw43_arch.h>
}

NetworkController::NetworkController() {}

void NetworkController::begin()
{
    WiFi.setHostname("IoT-Sensor-Alpha");
    WiFi.mode(WIFI_STA);
    delay(100);
}

// O(1) complexity - Reads from OTP memory/driver
String NetworkController::getPhysicalMac()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char buf[20];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

// O(1) complexity - Reads directly from active LwIP netif layer
String NetworkController::getActiveMac()
{
    cyw43_arch_lwip_begin();
    extern cyw43_t cyw43_state;
    uint8_t *m = cyw43_state.netif[0].hwaddr;
    char buf[20];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
    cyw43_arch_lwip_end();
    return String(buf);
}

void NetworkController::applyMacSpoofing(uint8_t *fakeMac)
{
    cyw43_arch_lwip_begin();
    extern cyw43_t cyw43_state;
    if (cyw43_state.netif != NULL)
    {
        memcpy(cyw43_state.netif[0].hwaddr, fakeMac, 6);
    }
    cyw43_arch_lwip_end();
}

void NetworkController::update(NetworkModel &model)
{
    // 1. IP Address Extraction
    String ip = WiFi.localIP().toString();
    // Safely copy string to char array preventing buffer overflow
    strncpy(model.ip_address, ip.c_str(), sizeof(model.ip_address) - 1);
    model.ip_address[sizeof(model.ip_address) - 1] = '\0';

    // 2. Radio Signal Strength Indicator (RSSI)
    model.signal_strength_rssi = WiFi.RSSI();

    // 3. Link Uptime Calculation
    if (WiFi.status() == WL_CONNECTED)
    {
        if (_connection_start_ms == 0)
        {
            _connection_start_ms = millis(); // Mark the establishment time
        }
        model.connection_uptime = (millis() - _connection_start_ms) / 1000; // in seconds
    }
    else
    {
        _connection_start_ms = 0;
        model.connection_uptime = 0;
    }
}