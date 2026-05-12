#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>

// --- HARDWARE PINOUT MAPPING ---
#define PIN_DHT_SENSOR 22  // GPIO22: 1-Wire data bus for DHT11 environmental sensor
#define PIN_SOIL_SENSOR A0 // GPIO26 (ADC0): Capacitive soil moisture sensor input
#define PIN_PUMP_RELAY 14  // GPIO14: Actuator control line for water pump relay

// --- SENSOR CALIBRATION MATRIX (12-bit Resolution: 0 - 4095) ---
// Derived from baseline voltages: Dry ~2.2V, Wet ~1.2V at 3.3V AREF
#define SOIL_AIR_DRY 710   // ADC baseline for 0% volumetric water content (ambient air)
#define SOIL_WATER_WET 382 // ADC baseline for 100% saturation (submerged)

// --- NETWORK COMMUNICATION PROTOCOL ---
// Target ingress server for UDP datagrams
const char *SERVER_HOSTNAME = "192.168.2.118"; // Destination IPv4 address
const uint16_t SERVER_PORT = 5005;             // Destination UDP port

#endif // APP_CONFIG_H