from Crypto.Cipher import ChaCha20_Poly1305
import logging

# Initialize logger for industrial telemetry monitoring
logger = logging.getLogger(__name__)

def decrypt_telemetry(raw_ingress_data: bytes, key_hex: str) -> str:
    """
    Decrypts and validates incoming IoT packets using ChaCha20-Poly1305 AEAD.
    
    Packet Architecture (Binary Format):
    - Bytes [0:4]:  32-bit Message Counter (Little-Endian)
    - Bytes [4:20]: 128-bit Authentication Tag (Poly1305)
    - Bytes [20:]:  Encrypted Payload (Ciphertext)
    """
    try:
        # Minimum packet size verification: 4 (Counter) + 16 (Tag) = 20 bytes
        if len(raw_ingress_data) < 20:
            logger.warning("Packet dropped: Incomplete header.")
            return None

        # 1. Component Extraction
        msg_counter_bytes = raw_ingress_data[:4]
        tag = raw_ingress_data[4:20]      
        ciphertext = raw_ingress_data[20:] 
        
        # 2. Nonce Reconstruction (96-bit: 4-byte Counter + 8-byte zero-padding)
        nonce = msg_counter_bytes + b'\x00' * 8
        
        # 3. Cryptographic Context Initialization
        key = bytes.fromhex(key_hex)
        cipher = ChaCha20_Poly1305.new(key=key, nonce=nonce)
        
        # 4. AEAD Decryption and Integrity Verification
        # Throws ValueError if the Authentication Tag is tampered or invalid
        decrypted_payload = cipher.decrypt_and_verify(ciphertext, tag)
        
        return decrypted_payload.decode('utf-8')
        
    except ValueError:
        logger.error("SECURITY ALERT: Integrity check failed! Potential tampering detected.")
        return None
    except Exception as e:
        logger.error(f"SYSTEM ERROR: Decryption engine failure: {str(e)}")
        return None