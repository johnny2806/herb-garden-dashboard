# FILE: server/tests/test_sender.py (Bản AEAD)
import socket
import time
from Crypto.Cipher import ChaCha20_Poly1305

UDP_IP = "127.0.0.1"
UDP_PORT = 5005
SECRET_KEY = "7365637265746b65793031323334353637383930313233343536373839303132"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
message_counter = 0

while True:
    message_counter += 1
    # Use Comma Separated Format: "ID:NODE-01,T:28.5,H:65.0,S:45.0"
    raw_data = f"ID:NODE-01,T:28.5,H:65.0,S:45.0"
    
    nonce = message_counter.to_bytes(4, 'little') + b'\x00' * 8
    key = bytes.fromhex(SECRET_KEY)
    
    # Use ChaCha20_Poly1305
    cipher = ChaCha20_Poly1305.new(key=key, nonce=nonce)
    ciphertext, tag = cipher.encrypt_and_digest(raw_data.encode('utf-8'))
    
    # Packet 63 bytes: [Counter(4)] + [Tag(16)] + [Ciphertext]
    packet = message_counter.to_bytes(4, 'little') + tag + ciphertext
    sock.sendto(packet, (UDP_IP, UDP_PORT))
    time.sleep(2)