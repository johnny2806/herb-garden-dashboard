#include "controllers/SecurityController.h"
#include <Arduino.h>
#include <string>

/**
 * @brief Constructor: Initializes the 256-bit key and message counter.
 * @param key Pointer to the master security key.
 */
SecurityController::SecurityController(const uint8_t *key) : _key(key), _msg_counter(0) {}

/**
 * @brief Encrypts telemetry using ChaCha20-Poly1305 (AEAD).
 * Protocol Structure: [Counter (4B)] + [Tag (16B)] + [Ciphertext (nB)]
 * * @param plaintext The raw sensor data string.
 * @return std::string Assembled binary datagram.
 */
std::string SecurityController::encryptPayload(const std::string &plaintext)
{
    // Atomic increment for unique nonce generation per packet
    _msg_counter++;

    // 1. Nonce Derivation: 4-byte Counter + 8-byte zero padding (96-bit total)
    uint8_t nonce[12] = {0};
    memcpy(nonce, &_msg_counter, sizeof(_msg_counter));

    // 2. Encryption & Authentication Tag Generation
    uint8_t *ciphertext = new uint8_t[plaintext.length()];
    uint8_t tag[16]; // Buffer for the 128-bit authentication tag

    _chacha.clear();
    _chacha.setKey(_key, 32); // Load 256-bit Master Key
    _chacha.setIV(nonce, 12); // Load 96-bit Nonce

    // Encrypt the payload
    _chacha.encrypt(ciphertext, (const uint8_t *)plaintext.c_str(), plaintext.length());
    // Generate the Poly1305 Integrity Tag
    _chacha.computeTag(tag, 16);

    // 3. Binary Packet Assembly
    std::string final_packet;

    // Header: 4-byte Message Sequence
    final_packet.append((char *)&_msg_counter, sizeof(_msg_counter));

    // Security: 16-byte Integrity Check (Poly1305 Tag)
    final_packet.append((char *)tag, 16);

    // Payload: Encrypted Telemetry Data
    final_packet.append((char *)ciphertext, plaintext.length());

    // Memory Management: Release dynamic buffer
    delete[] ciphertext;

    return final_packet;
}