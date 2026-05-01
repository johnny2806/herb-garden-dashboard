#include "controllers/SecurityController.h"

SecurityController::SecurityController(const uint8_t* key) : _key(key) {}

void SecurityController::encrypt(const String& plaintext, uint8_t* output, uint8_t* tag, uint8_t* iv) {
    _chacha.clear();
    _chacha.setKey(_key, 32);
    _chacha.setIV(iv, 12); // Standard 12-byte Nonce
    _chacha.encrypt(output, (const uint8_t*)plaintext.c_str(), plaintext.length());
    _chacha.computeTag(tag, 16); // Create authentication tag
}