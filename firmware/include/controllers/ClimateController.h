#ifndef CLIMATE_CONTROLLER_H
#define CLIMATE_CONTROLLER_H

#include <DHT.h>
#include "models.h"

class ClimateController {
public:
    ClimateController(uint8_t pin, uint8_t type);
    void begin();
    void update(ClimateModel &model); // Update micro-climate data into the model

private:
    DHT _dht;
};

#endif