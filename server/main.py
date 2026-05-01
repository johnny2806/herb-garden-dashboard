import os
import threading
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from dotenv import load_dotenv

from app.core.security import Decryptor
from app.services.telemetry import TelemetryService

# --- CONFIGURATION LAYER ---
# Load environment variables from .env file
load_dotenv() 

app = FastAPI(title="Secure Herb Garden Gateway")

# Enable Cross-Origin Resource Sharing for the Web Dashboard
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize Security Layer with Environment Key
# Default fallback is provided for development only
SECRET_KEY_HEX = os.getenv("CHACHA20_SECRET_KEY", "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f")
SHARED_KEY = bytes.fromhex(SECRET_KEY_HEX)

# Service Instances
decryptor = Decryptor(SHARED_KEY)
telemetry_service = TelemetryService()

# --- LIFECYCLE HOOKS ---
@app.on_event("startup")
def start_background_workers():
    """Spawn the UDP listener on a dedicated daemon thread."""
    worker = threading.Thread(
        target=telemetry_service.start_listener, 
        args=(decryptor,), 
        daemon=True
    )
    worker.start()

# --- REST ENDPOINTS ---
@app.get("/api/v1/health")
async def get_health_status():
    """System health check endpoint."""
    return {"status": "operational", "engine": "FastAPI"}

@app.get("/api/v1/telemetry/latest")
async def fetch_telemetry():
    """
    Retrieves the most recent verified datagram.
    Response Time: O(1)
    """
    return telemetry_service.get_latest_telemetry()