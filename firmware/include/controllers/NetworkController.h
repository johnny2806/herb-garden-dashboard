#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include <Arduino.h>
#include "models.h"

/**
 * @class NetworkController
 * @brief Handles MAC address spoofing and hardware identity management.
 */
class NetworkController
{
public:
    NetworkController();
    void begin();

    /** @brief Retrieves the immutable factory-set MAC address. */
    String getPhysicalMac();

    /** @brief Retrieves the current active MAC address (spoofed or real). */
    String getActiveMac();

    /** @brief Applies locally administered MAC address (LAA) for stealth operations. */
    void applyMacSpoofing(uint8_t *fakeMac);

    /** * @brief Extracts real-time network metrics into the domain model.
     * @param model Reference to the NetworkModel structure.
     */
    void update(NetworkModel &model);

private:
    uint32_t _connection_start_ms; // Tracks when the WLAN link was established
};

#endif