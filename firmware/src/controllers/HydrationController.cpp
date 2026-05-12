/*
 * @file HydrationController.cpp
 * @brief Soil moisture sensor interface utilizing Native Pico SDK Bypass.
 */

#include "controllers/HydrationController.h"
#include <Arduino.h>

// Low-level Hardware SDK to mitigate Arduino Core & CYW43 WLAN conflicts
#include "hardware/adc.h"
#include "hardware/gpio.h"

HydrationController::HydrationController(uint8_t pin, uint16_t dryLimit, uint16_t wetLimit)
    : _pin(pin), _dryLimit(dryLimit), _wetLimit(wetLimit) {}

void HydrationController::begin()
{
    // Initialize the underlying hardware ADC subsystem
    adc_init();

    // Prepare the specified GPIO pin exclusively for analog input (strips digital buffers)
    adc_gpio_init(_pin);
}

void HydrationController::update(SoilHydrationModel &model)
{
    // Map GPIO pin (26, 27, 28) to hardware ADC Channel (0, 1, 2)
    uint8_t adc_channel = _pin - 26;

    // Lock the multiplexer to the target channel and perform 12-bit sampling
    adc_select_input(adc_channel);
    uint16_t raw = adc_read();

    model.raw_value = raw;

    // Linear interpolation mapping raw ADC to saturation percentage
    float pct = (float)(_dryLimit - raw) / (_dryLimit - _wetLimit) * 100.0;

    // Clamp the output to prevent logical overflow (0.0% to 100.0%)
    model.saturation_percentage = constrain(pct, 0.0, 100.0);
}