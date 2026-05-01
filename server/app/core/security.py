from Crypto.Cipher import ChaCha20
import logging

# Initialize logger for security events
logger = logging.getLogger(__name__)

def decrypt_telemetry(raw_ingress_data: bytes, key_hex: str) -> str:
    """
    Decrypts incoming UDP telemetry packets using ChaCha20 stream cipher.
    
    Architecture:
    - Bytes [0:4]: 32-bit Message Counter (used as Nonce/IV)
    - Bytes [4:]: Encrypted Payload (Ciphertext)
    """
    try:
        # 1. Extract the 32-bit message counter from the ingress buffer
        # This counter must match the value incremented by the Pico W firmware
        msg_counter_bytes = raw_ingress_data[:4]
        ciphertext = raw_ingress_data[4:]
        
        # 2. Reconstruct the 96-bit (12-byte) Nonce
        # We append 8 bytes of zero-padding to the 4-byte counter (Little-Endian)
        nonce = msg_counter_bytes + b'\x00' * 8
        
        # 3. Initialize the ChaCha20 cipher context with the 256-bit key
        key = bytes.fromhex(key_hex)
        cipher = ChaCha20.new(key=key, nonce=nonce)
        
        # 4. Decrypt and decode the remaining payload
        decrypted_payload = cipher.decrypt(ciphertext)
        return decrypted_payload.decode('utf-8')
        
    except Exception as e:
        logger.error(f"Decryption Failure: Integrity check failed or malformed packet. Error: {str(e)}")
        return None