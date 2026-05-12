#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>

// --- HARDWARE PINOUT MAPPING ---
#define PIN_DHT_SENSOR 22  // GPIO22: 1-Wire data bus for DHT11 environmental sensor
#define PIN_SOIL_SENSOR 26 // GPIO26 (ADC0): Capacitive soil moisture sensor (Native SDK Bypass)
#define PIN_PUMP_RELAY 14  // GPIO14: Actuator control line for water pump relay

// --- SENSOR CALIBRATION MATRIX (Native 12-bit SDK Resolution: 0 - 4095) ---
// Derived from baseline hardware telemetry: Dry ~2.28V, Wet ~1.25V at 3.3V AREF
#define SOIL_AIR_DRY 2833   // ADC baseline for 0% volumetric water content (ambient air)
#define SOIL_WATER_WET 1553 // ADC baseline for 100% saturation (submerged)

// --- NETWORK COMMUNICATION PROTOCOL ---
// Target ingress server for UDP datagrams
const char *SERVER_HOSTNAME = "192.168.2.118"; // Destination IPv4 address
const uint16_t SERVER_PORT = 5005;             // Destination UDP port

#endif // APP_CONFIG_H