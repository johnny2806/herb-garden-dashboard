/**
 * @file models.h
 * @author Exploratory Nerd - Universal Polymath
 * @brief Unified Domain Models for the Herb Garden Node Project.
 * @version 1.1.0
 * @standard US English, Clean Code, US Technical Standards.
 */

#ifndef HERB_GARDEN_MODELS_H
#define HERB_GARDEN_MODELS_H

#include <Arduino.h>

/**
 * 1. Climate Model: Captures environmental data from the DHT11 sensor.
 * (Used in Task 1 Main Logic)
 */
struct ClimateModel
{
    float temperature_celsius; // Current air temperature
    float humidity_percentage; // Current air humidity
    bool is_valid;             // Sensor data integrity flag
};

/**
 * 2. Soil Hydration Model: Represents the moisture state of the herb plant.
 * (Used in Task 1 Main Logic)
 */
struct SoilHydrationModel
{
    uint16_t raw_value;          // Direct ADC reading from V1.2 sensor (0-1024)
    float saturation_percentage; // Calculated moisture level (0.0 to 100.0)
};

/**
 * 3. Actuator Model: Tracks the state and health of the Pump/Relay.
 */
struct ActuatorModel
{
    bool is_active;               // Current relay physical state
    uint32_t last_activation_ms;  // Timestamp of the last run
    uint32_t session_runtime_sec; // Current or last run duration
};

/**
 * 4. Network Model: Diagnostics for the Pico W WiFi subsystem.
 * Ref:
 */
struct NetworkModel
{
    char ip_address[16];          // Local IP assigned by DHCP
    int32_t signal_strength_rssi; // Signal quality in dBm
    uint32_t connection_uptime;   // Duration of the current WiFi session
};

/**
 * 5. System Health Model: Internal RP2040 chip diagnostics.
 * (Used in Task 1 Main Logic)
 */
struct SystemHealthModel
{
    float core_temp;          // Internal chip temperature sensor
    uint32_t uptime_ms;       // Milliseconds since system boot
    uint32_t free_heap_bytes; // Available memory (RAM)
};

/**
 * 6. Configuration Model: Automation thresholds and user settings.
 */
struct ConfigurationModel
{
    float moisture_threshold;    // Trigger point for auto-watering
    uint16_t watering_limit_sec; // Maximum allowed pump runtime
    uint32_t telemetry_rate_ms;  // Backend synchronization frequency
};

/**
 * 7. Calibration Model: Reference points for sensor accuracy.
 */
struct CalibrationModel
{
    uint16_t air_dry_value;   // Sensor reading when completely dry
    uint16_t water_wet_value; // Sensor reading when submerged in water
};

/**
 * 8. Event Log Model: Structures system errors and alerts.
 */
struct EventLogModel
{
    uint16_t event_id;    // Unique identifier for error types
    char description[48]; // Brief US-English event description
    uint32_t timestamp;   // Occurance time
};

/**
 * 9. Herb Metadata Model: Static information about the plant species.
 */
struct HerbMetadataModel
{
    char species_name[24]; // E.g., "Sweet Basil", "Mint"
    float target_temp_min; // Ideal minimum temperature
    float target_temp_max; // Ideal maximum temperature
};

/**
 * 10. Telemetry Packet: Master wrapper for REST API serialization.
 * (Designed for ArduinoJson integration)
 */
struct TelemetryPacket
{
    ClimateModel climate;
    SoilHydrationModel hydration;
    SystemHealthModel health;
    ActuatorModel actuator;
    uint32_t sequence_id; // Packet counter to track data loss
};

#endif // HERB_GARDEN_MODELS_H