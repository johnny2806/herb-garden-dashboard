/*
 * @file SystemHealthController.cpp
 * @brief Monitors RP2040 hardware vitals using Native SDK.
 */

#include "controllers/SystemHealthController.h"
#include <hardware/adc.h>

SystemHealthController::SystemHealthController() {}

void SystemHealthController::begin()
{
    // Initialize ADC hardware and enable the internal temperature sensor (Channel 4)
    adc_init();
    adc_set_temp_sensor_enabled(true);
}

void SystemHealthController::update(SystemHealthModel &model)
{
    // 1. Silicon Temperature Acquisition
    // Select ADC Input 4 (Internal Temperature Sensor)
    adc_select_input(4);
    uint16_t raw_adc = adc_read();

    // Convert 12-bit raw value to voltage (3.3V reference)
    const float conversion_factor = 3.3f / (1 << 12);
    float voltage = raw_adc * conversion_factor;

    /**
     * RP2040 Temperature Conversion Formula (Native SDK Standard):
     * T = 27 - (Voltage - 0.706) / 0.001721
     * Removed manual offset to provide raw silicon temperature telemetry.
     */
    model.core_temp = 27.0f - (voltage - 0.706f) / 0.001721f;

    // 2. System Uptime Tracking
    model.uptime_ms = millis();

    // 3. Memory Diagnostics
    // Retrieve free heap space using RP2040 Arduino Core helper
    model.free_heap_bytes = rp2040.getFreeHeap();
}