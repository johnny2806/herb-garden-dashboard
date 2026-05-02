"""
Herb Garden Secure Ingress Server
Architecture: FastAPI + UDP Ingress Worker (AEAD Verified)
Standard: US Industrial IoT Security Protocol
"""

import socket
import threading
import os
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.core.security import decrypt_telemetry
from dotenv import load_dotenv

# --- SYSTEM INITIALIZATION ---
load_dotenv()
CHACHA_KEY = os.getenv("CHACHA_KEY")
UDP_IP = "0.0.0.0"
UDP_PORT = 5005

app = FastAPI(title="Herb Garden Secure Ingress")

# Enable Cross-Origin Resource Sharing (CORS) for external dashboard access
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# Global state for thread-safe telemetry storage
latest_telemetry = {
    "status": "OFFLINE",
    "verified": False,
    "identity": "--:--:--:--:--:--",
    "temperature_celsius": 0.00,
    "humidity_percentage": 0.00,
    "saturation_percentage": 0.00
}

def udp_ingress_worker():
    """Background worker for asynchronous encrypted UDP ingestion."""
    global latest_telemetry
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    print(f"[SYSTEM] Secure Ingress Node Active on {UDP_IP}:{UDP_PORT}")

    while True:
        try:
            data, addr = sock.recvfrom(1024)
            # Decrypt 63-byte AEAD datagrams from the hardware node
            decrypted_str = decrypt_telemetry(data, CHACHA_KEY)
            
            if decrypted_str:
                # FIX: split(':', 1) preserves the full MAC address by splitting only at the first colon
                parts = {kv.split(':', 1)[0]: kv.split(':', 1)[1] for kv in decrypted_str.split(',')}
                
                # Update global telemetry with parsed and verified metrics
                latest_telemetry = {
                    "status": "ONLINE",
                    "verified": True,
                    "identity": parts.get("ID", "UNKNOWN"),
                    "temperature_celsius": float(parts.get("T", 0)),
                    "humidity_percentage": float(parts.get("H", 0)),
                    "saturation_percentage": float(parts.get("S", 0))
                }
                print(f"[INGEST] Valid Frame Decoded from {addr[0]}")
        except Exception as e:
            print(f"[ERROR] Ingress processing failed: {e}")

# Dispatch the UDP ingestion worker to a background daemon thread
threading.Thread(target=udp_ingress_worker, daemon=True).start()

@app.get("/")
async def root():
    """Ingress server health status check."""
    return {"status": "active", "service": "Herb-Garden-Security-Ingress"}

@app.get("/api/v1/telemetry/latest")
async def get_telemetry():
    """REST API endpoint for synchronous dashboard updates."""
    return latest_telemetry

if __name__ == "__main__":
    import uvicorn
    # Execute the ASGI server on the local network interface
    uvicorn.run(app, host="0.0.0.0", port=8000)