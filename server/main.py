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
from typing import Optional

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
    "saturation_percentage": 0.00,
    "pump_is_active": False
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
                print(f"[DEBUG INGRESS]: {decrypted_str}")
                parts = {kv.split(':', 1)[0]: kv.split(':', 1)[1] for kv in decrypted_str.split(',')}
                
                # Update global telemetry with parsed and verified metrics
                latest_telemetry = {
                    "status": "ONLINE",
                    "verified": True,
                    "identity": parts.get("ID", "UNKNOWN"),
                    "temperature_celsius": float(parts.get("T", 0)),
                    "humidity_percentage": float(parts.get("H", 0)),
                    "saturation_percentage": float(parts.get("S", 0)),
                    "soil_raw_adc": int(parts.get("SR", 0)),
                    "core_temp": float(parts.get("CT", 0)),
                    "free_heap_bytes": int(parts.get("MEM", 0)),
                    "rssi_dbm": int(parts.get("RSSI", 0)),
                    "uptime_sec": int(parts.get("UP", 0)),
                    "pump_is_active": bool(int(parts.get("PMP", 0))) # Extract relay logic state
                }
                print(f"[INGEST] Valid Frame Decoded from {addr[0]}")
        except Exception as e:
            print(f"[ERROR] Ingress processing failed: {e}")

# Dispatch the UDP ingestion worker to a background daemon thread
threading.Thread(target=udp_ingress_worker, daemon=True).start()

pending_command = {"pump": None} # None: Auto, True: Force ON, False: Force OFF

@app.post("/api/v1/control/pump")
async def control_pump(state: Optional[str] = None):
    """Manual override endpoint. Accepts 'true', 'false', or 'null' (Auto)."""
    global pending_command
    
    # Parse the string state from JS into Python boolean/None
    if state == "true":
        pending_command["pump"] = True
    elif state == "false":
        pending_command["pump"] = False
    else:
        pending_command["pump"] = None
        
    return {"status": "COMMAND_QUEUED", "state": pending_command["pump"]}

@app.get("/")
async def root():
    """Ingress server health status check."""
    return {"status": "active", "service": "Herb-Garden-Security-Ingress"}

@app.get("/api/v1/telemetry/latest")
async def get_telemetry():
    """REST API endpoint for synchronous dashboard updates."""
    return {
        "telemetry": latest_telemetry,
        "command": pending_command["pump"]
    }

if __name__ == "__main__":
    import uvicorn
    # Execute the ASGI server on the local network interface
    uvicorn.run(app, host="0.0.0.0", port=8000)