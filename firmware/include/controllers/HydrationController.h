#ifndef HYDRATION_CONTROLLER_H
#define HYDRATION_CONTROLLER_H

#include <Arduino.h>
#include "models.h"

class HydrationController {
public:
    // Khởi tạo với chân cắm và các giá trị hiệu chuẩn
    HydrationController(uint8_t pin, uint16_t dryLimit, uint16_t wetLimit);
    void begin();
    void update(SoilHydrationModel &model); // Cập nhật dữ liệu vào model

private:
    uint8_t _pin;
    uint16_t _dryLimit;
    uint16_t _wetLimit;
};

#endif