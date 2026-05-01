#include "controllers/SecurityController.h"
#include "Secrets.h"
#include <Arduino.h>
#include "Crypto.h"
#include "ChaCha.h"

SecurityController::SecurityController(const uint8_t *key) : _msg_counter(0) {}

void SecurityController::encrypt(const String &plaintext, uint8_t *output, uint8_t *tag, uint8_t *iv)
{
    _chacha.clear();
    _chacha.setKey(_key, 32);
    _chacha.setIV(iv, 12); // Standard 12-byte Nonce
    _chacha.encrypt(output, (const uint8_t *)plaintext.c_str(), plaintext.length());
    _chacha.computeTag(tag, 16); // Create authentication tag
}

std::string SecurityController::encryptPayload(const std::string &ciphertext)
{
    _msg_counter++; // Tăng số đếm sau mỗi lần gửi gói tin

    uint8_t nonce[12] = {0};

    memcpy(nonce, &_msg_counter, sizeof(_msg_counter));

    std::string final_packet;
    final_packet.append((char *)&_msg_counter, sizeof(_msg_counter));
    final_packet.append(ciphertext);

    return final_packet;
}