/*
 * @file ActuatorController.h
 * @brief Actuator interface for controlling the water pump/relay.
 * @standard US English, Clean Code.
 */

#ifndef ACTUATOR_CONTROLLER_H
#define ACTUATOR_CONTROLLER_H

#include <Arduino.h>
#include "models.h"

class ActuatorController
{
public:
    /*
     * @brief Constructor mapping the hardware GPIO pin for the relay.
     * @param pin GPIO pin number.
     */
    ActuatorController(uint8_t pin);

    // Initializes the GPIO using Native SDK
    void begin();

    // Updates the ActuatorModel with current hardware state
    void update(ActuatorModel &model);

    // Command interface to engage the relay (Turn ON)
    void activate();

    // Command interface to disengage the relay (Turn OFF)
    void deactivate();

    // Add to public methods
    void set_state(bool state);

private:
    uint8_t _pin;
    bool _current_state;
    uint32_t _last_activation_ms;
};

#endif // ACTUATOR_CONTROLLER_H