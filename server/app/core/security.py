from Crypto.Cipher import ChaCha20_Poly1305
import binascii

class Decryptor:
    """Handles ChaCha20-Poly1305 AEAD decryption O(n)."""
    def __init__(self, master_key: bytes):
        if len(master_key) != 32:
            raise ValueError("ChaCha20 Key must be 32 bytes.")
        self.key = master_key

    def decrypt(self, nonce_hex: str, ciphertext_hex: str) -> str:
        try:
            nonce = bytes.fromhex(nonce_hex.replace(" ", ""))
            ciphertext = bytes.fromhex(ciphertext_hex.replace(" ", ""))
            cipher = ChaCha20_Poly1305.new(key=self.key, nonce=nonce)
            return cipher.decrypt(ciphertext).decode('utf-8')
        except (ValueError, TypeError) as e:
            return f"Decryption_Error: {str(e)}"