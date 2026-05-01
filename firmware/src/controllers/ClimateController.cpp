#include "controllers/ClimateController.h"
#include <Arduino.h>

// Constructor initializes the DHT sensor with the specified pin and type
ClimateController::ClimateController(uint8_t pin, uint8_t type) : _dht(pin, type) {}

void ClimateController::begin() {
    _dht.begin();
}

void ClimateController::update(ClimateModel &model) {
    float h = _dht.readHumidity();
    float t = _dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        model.is_valid = false;
        return;
    }

    model.humidity_percentage = h;
    model.temperature_celsius = t;
    model.is_valid = true;
}