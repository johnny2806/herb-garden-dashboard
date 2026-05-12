#ifndef HYDRATION_CONTROLLER_H
#define HYDRATION_CONTROLLER_H

#include <Arduino.h>
#include "models.h"

class HydrationController
{
public:
    /*
     * @brief Constructor mapping hardware parameters and calibration limits.
     * @param pin GPIO pin number designated for ADC.
     * @param dryLimit 12-bit ADC baseline for 0% moisture.
     * @param wetLimit 12-bit ADC baseline for 100% moisture.
     */
    HydrationController(uint8_t pin, uint16_t dryLimit, uint16_t wetLimit);

    // Initializes the Native Pico SDK ADC subsystem
    void begin();

    // Acquires hardware data and updates the corresponding model
    void update(SoilHydrationModel &model);

private:
    uint8_t _pin;
    uint16_t _dryLimit;
    uint16_t _wetLimit;
};

#endif // HYDRATION_CONTROLLER_H