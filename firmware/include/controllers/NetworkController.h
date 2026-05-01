#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include <Arduino.h>

class NetworkController {
public:
    NetworkController();
    void begin();
    String getPhysicalMac();  // Lấy MAC thật của chip
    String getActiveMac();    // Lấy MAC đang chạy (có thể là giả)
    void applyMacSpoofing(uint8_t* fakeMac);
};

#endif