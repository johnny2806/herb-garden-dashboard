#include "controllers/HydrationController.h"
#include <Arduino.h>

/**
 * @brief Controller for managing soil hydration sensing and data processing.
 * @details This controller reads raw ADC values from the soil moisture sensor,
 * @cite uploaded:firmware/include/controllers/HydrationController.h
 */
HydrationController::HydrationController(uint8_t pin, uint16_t dryLimit, uint16_t wetLimit)
    : _pin(pin), _dryLimit(dryLimit), _wetLimit(wetLimit) {}

/**
 * @brief Initializes the hardware for the soil moisture sensor.
 */
void HydrationController::begin()
{
    pinMode(_pin, INPUT);
}

/**
 * @brief Reads the raw ADC value from the soil moisture sensor and calculates the saturation percentage.
 * @cite uploaded:HydrationController.cpp
 */
void HydrationController::update(SoilHydrationModel &model)
{
    // 1. Read raw ADC value from the soil moisture sensor
    uint16_t raw = analogRead(_pin);
    model.raw_value = raw;

    // 2. US Standard: Linear Interpolation Formula
    // Percentage = ((Dry - Current) / (Dry - Wet)) * 100
    float pct = (float)(_dryLimit - raw) / (_dryLimit - _wetLimit) * 100.0;

    // 3. Constrain the result to 0-100% to avoid errors
    model.saturation_percentage = constrain(pct, 0.0, 100.0);
}