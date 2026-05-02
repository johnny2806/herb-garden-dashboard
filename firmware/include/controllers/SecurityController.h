#ifndef SECURITY_CONTROLLER_H
#define SECURITY_CONTROLLER_H

#include <Arduino.h>
#include <string>
#include <ChaChaPoly.h>

/**
 * @class SecurityController
 * @brief High-security AEAD orchestrator using ChaCha20-Poly1305.
 */
class SecurityController
{
public:
    SecurityController(const uint8_t *key);

    /**
     * @brief Encapsulates telemetry into a secure binary datagram.
     * Protocol: [4B Counter] + [16B Auth Tag] + [nB Ciphertext]
     */
    std::string encryptPayload(const std::string &plaintext);

private:
    const uint8_t *_key;
    ChaChaPoly _chacha;
    uint32_t _msg_counter;
};

#endif