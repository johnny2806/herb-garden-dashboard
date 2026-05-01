#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Hardware Pins
#define PIN_DHT_SENSOR 22  // DHT11 Data Pin (GPIO22)
#define PIN_SOIL_SENSOR 26 // Soil Moisture Sensor Pin (GPIO26)
#define PIN_PUMP_RELAY 14  // Water Pump Relay Pin (GPIO14)

// Calibration Values
#define SOIL_AIR_DRY 700   // Read when soil is completely dry (0% moisture)
#define SOIL_WATER_WET 360 // Read when soil is fully saturated (100% moisture)

// --- NETWORK CONFIGURATION (US English) ---
// Find your computer's LAN IP when connected to the mobile hotspot (e.g., 192.168.43.10)
const char *SERVER_HOSTNAME = "192.168.2.118"; // CHANGE THIS TO YOUR COMPUTER'S IP
const uint16_t SERVER_PORT = 5005;             // Port number for UDP communication

#endif