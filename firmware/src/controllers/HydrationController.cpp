/**
 * @file HydrationController.cpp
 * @brief Soil moisture sensor interface implementation.
 */

#include "controllers/HydrationController.h"
#include <Arduino.h>

/**
 * @brief Constructor mapping hardware parameters.
 */
HydrationController::HydrationController(uint8_t pin, uint16_t dryLimit, uint16_t wetLimit)
    : _pin(pin), _dryLimit(dryLimit), _wetLimit(wetLimit) {}

void HydrationController::begin()
{
    pinMode(_pin, INPUT); // Initialize ADC pin for input
}

/**
 * @brief Performs ADC sampling and converts raw data to saturation percentage.
 */
void HydrationController::update(SoilHydrationModel &model)
{
    uint16_t raw = analogRead(_pin);
    model.raw_value = raw;

    // Standard linear interpolation for moisture percentage
    float pct = (float)(_dryLimit - raw) / (_dryLimit - _wetLimit) * 100.0;
    model.saturation_percentage = constrain(pct, 0.0, 100.0);
}