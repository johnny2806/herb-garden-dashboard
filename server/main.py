import socket
import threading
from fastapi import FastAPI
from app.core.security import decrypt_telemetry
from dotenv import load_dotenv
import os

# Load environment variables from .env file
load_dotenv()
CHACHA_KEY = os.getenv("CHACHA_KEY")
UDP_IP = "0.0.0.0"  # Listen on all available network interfaces
UDP_PORT = 5005

app = FastAPI(title="Herb Garden Ingestion Server")

# Thread-safe storage for the latest decrypted telemetry packet
latest_telemetry = {"data": "AWAITING_INGESTION", "timestamp": "N/A"}

def udp_ingress_worker():
    """
    Background worker thread to handle asynchronous UDP packet ingestion.
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    print(f"[SYSTEM] UDP Ingress Port {UDP_PORT} is now ACTIVE.")

    while True:
        # Standard UDP buffer size for telemetry payloads
        data, addr = sock.recvfrom(1024)
        
        # Execute synchronized decryption logic
        decrypted_str = decrypt_telemetry(data, CHACHA_KEY)
        
        if decrypted_str:
            global latest_telemetry
            latest_telemetry = {
                "data": decrypted_str,
                "origin": addr[0]
            }
            # Technical log following US standard instrumentation format
            print(f"[INGEST] Valid packet received from {addr[0]} | Payload: {decrypted_str}")

# Initialize ingress worker thread
threading.Thread(target=udp_ingress_worker, daemon=True).start()

@app.get("/api/telemetry")
async def get_telemetry():
    """
    Exposes the latest telemetry data via a RESTful endpoint for the Dashboard.
    """
    return latest_telemetry