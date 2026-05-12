#ifndef SYSTEM_HEALTH_CONTROLLER_H
#define SYSTEM_HEALTH_CONTROLLER_H

#include <Arduino.h>
#include "models.h"

/*
 * @class SystemHealthController
 * @brief Internal hardware diagnostic engine for the RP2040 silicon.
 * Monitors core temperature, heap memory allocation, and system uptime.
 */
class SystemHealthController
{
public:
    SystemHealthController();

    /* @brief Initializes the onboard ADC temperature sensor. */
    void begin();

    /* * @brief Polls internal hardware registers to populate the health model.
     * @param model Reference to the SystemHealthModel structure.
     */
    void update(SystemHealthModel &model);
};

#endif