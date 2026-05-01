# FILE: test_sender.py
# Purpose: Simulates a Pico W sending encrypted telemetry via UDP.
# US English Terminology: Mocking, Payload, Datagram.

import socket
import time

# 1. Configuration (Match your Server settings)
SERVER_IP = "127.0.0.1"  # Localhost for testing
SERVER_PORT = 5005
# Copy an actual IV and Ciphertext from your previous Serial Monitor logs
MOCK_IV = "2C 3A 94 98 19 74 93 1A EC E9 8C FF"
MOCK_CIPHER = "9E A2 FA 8C AC 59 38 BF 86 8C E5 7C 2D 4B 71 93 10 F7 E7 46 F6 13 EB A0 6C 78 B1 6B F8 DD 85 CD F4 9A 8F"

def send_mock_telemetry():
    """
    Constructs a UDP datagram and broadcasts it to the backend.
    Complexity: O(1) - Constant time transmission.
    """
    # Construct the protocol string: "IV:<hex>|CIPHER:<hex>"
    payload = f"IV:{MOCK_IV}|CIPHER:{MOCK_CIPHER}"
    
    # Create UDP Socket
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        print(f"[Mock] Injecting encrypted payload into {SERVER_IP}:{SERVER_PORT}...")
        sock.sendto(payload.encode('utf-8'), (SERVER_IP, SERVER_PORT))
        print("[Mock] Transmission successful.")

if __name__ == "__main__":
    send_mock_telemetry()