#ifndef SECURITY_CONTROLLER_H
#define SECURITY_CONTROLLER_H

#include <ChaChaPoly.h>
#include <Arduino.h>
#include <string>

class SecurityController
{
public:
    SecurityController(const uint8_t *key);
    void encrypt(const String &plaintext, uint8_t *output, uint8_t *tag, uint8_t *iv);
    String decrypt(const uint8_t *ciphertext, size_t len, const uint8_t *tag, const uint8_t *iv);
    SecurityController();
    std::string encryptPayload(const std::string &plaintext);

private:
    const uint8_t *_key;
    ChaChaPoly _chacha;
    uint32_t _msg_counter;

    struct SecurePacket
    {
        uint32_t counter;
        uint8_t payload[128]; // Adjust size as needed
    };
};

#endif