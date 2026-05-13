/*
 * @file ActuatorController.cpp
 * @brief Water pump/relay control implementation using Native Pico SDK.
 */

#include "controllers/ActuatorController.h"
#include <Arduino.h>

// Low-level Hardware SDK for precise GPIO control
#include "hardware/gpio.h"

ActuatorController::ActuatorController(uint8_t pin)
    : _pin(pin), _current_state(false), _last_activation_ms(0) {}

void ActuatorController::begin()
{
    // Initialize the GPIO pin explicitly bypassing Arduino Core
    gpio_init(_pin);

    // Set pin direction to OUTPUT
    gpio_set_dir(_pin, GPIO_OUT);

    // Enforce a fail-safe boot state (Relay Disengaged / OFF)
    gpio_put(_pin, 0);
}

void ActuatorController::update(ActuatorModel &model)
{
    // Mirror the internal state to the operational model
    model.is_active = _current_state;
    model.last_activation_ms = _last_activation_ms;
}

void ActuatorController::activate()
{
    if (!_current_state)
    {
        // Assert logic HIGH to engage the relay
        gpio_put(_pin, 1);
        _current_state = true;
        _last_activation_ms = millis();
        Serial.println(F("[ACTUATOR] INFO: Pump engaged."));
    }
}

void ActuatorController::deactivate()
{
    if (_current_state)
    {
        // Assert logic LOW to disengage the relay
        gpio_put(_pin, 0);
        _current_state = false;
        Serial.println(F("[ACTUATOR] INFO: Pump disengaged."));
    }
}

/*
 * @brief Forcefully sets the actuator state regardless of previous condition.
 * @param state Target state (true for ON, false for OFF).
 */
void ActuatorController::set_state(bool state)
{
    _current_state = state;
    gpio_put(_pin, _current_state ? 1 : 0);
    if (_current_state)
        _last_activation_ms = millis();
    Serial.printf("[ACTUATOR] Manual override: %s\n", _current_state ? "ON" : "OFF");
}