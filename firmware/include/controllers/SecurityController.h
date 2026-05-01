#ifndef SECURITY_CONTROLLER_H
#define SECURITY_CONTROLLER_H

#include <ChaChaPoly.h>
#include <Arduino.h>

class SecurityController {
public:
    SecurityController(const uint8_t* key);
    void encrypt(const String& plaintext, uint8_t* output, uint8_t* tag, uint8_t* iv);
    String decrypt(const uint8_t* ciphertext, size_t len, const uint8_t* tag, const uint8_t* iv);
private:
    const uint8_t* _key;
    ChaChaPoly _chacha;
};

#endif