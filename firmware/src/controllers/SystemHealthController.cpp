#include "controllers/SystemHealthController.h"
#include <hardware/adc.h>

SystemHealthController::SystemHealthController() {}

void SystemHealthController::begin()
{
    // Enable the onboard temperature sensor via RP2040 Hardware ADC (Channel 4)
    adc_init();
    adc_set_temp_sensor_enabled(true);
}

void SystemHealthController::update(SystemHealthModel &model)
{
    // 1. Silicon Temperature Calculation
    adc_select_input(4);
    uint16_t raw_adc = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    float voltage = raw_adc * conversion_factor;

    // Standard RP2040 temperature conversion formula
    // T = 27 - (ADC_voltage - 0.706) / 0.001721
    float raw_temp = 27.0f - (voltage - 0.706f) / 0.001721f;

    float temperature_offset = 57.0f; // Offset 57°C to align with external DHT11 reading
    model.core_temp = raw_temp - temperature_offset;

    // 2. System Uptime Tracking
    model.uptime_ms = millis();

    // 3. Memory Diagnostics (Heap Allocation)
    model.free_heap_bytes = rp2040.getFreeHeap();
}