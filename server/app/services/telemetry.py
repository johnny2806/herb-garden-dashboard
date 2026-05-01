import socket
import threading
import time
from typing import Dict, Any

class TelemetryService:
    """
    UDP Ingestion Layer for secure IoT telemetry.
    Thread-safe implementation with O(1) state access.
    """
    def __init__(self, host: str = "0.0.0.0", port: int = 5005):
        self.host = host
        self.port = port
        self._latest_record: Dict[str, Any] = {"status": "Awaiting ingress..."}
        self._highest_seq: int = 0
        self._lock = threading.Lock()

    def get_latest_telemetry(self) -> Dict[str, Any]:
        """Atomic read operation with O(1) complexity."""
        with self._lock:
            return self._latest_record.copy()

    def start_listener(self, decryptor):
        """Standard UDP server loop with Replay Attack prevention."""
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((self.host, self.port))
        print(f"[Service] Listening for UDP datagrams on {self.host}:{self.port}")

        while True:
            data, addr = sock.recvfrom(1024)
            try:
                # O(n) decryption complexity where n is payload length
                raw_msg = data.decode('utf-8')
                if "IV:" in raw_msg and "CIPHER:" in raw_msg:
                    parts = raw_msg.split("|")
                    iv = parts[0].split(":")[1]
                    cipher = parts[1].split(":")[1]
                    
                    decrypted = decryptor.decrypt(iv, cipher)
                    
                    # Anti-Replay Logic: Verify Sequence Number
                    # Expected format: "SEQ:<int>,ID:<mac>,..."
                    metadata = {kv.split(":")[0]: kv.split(":")[1] for kv in decrypted.split(",")}
                    current_seq = int(metadata.get("SEQ", 0))

                    with self._lock:
                        if current_seq > self._highest_seq:
                            self._highest_seq = current_seq
                            self._latest_record = {
                                "client_ip": addr[0],
                                "decrypted_payload": decrypted,
                                "timestamp": time.time(),
                                "seq": current_seq
                            }
                        else:
                            print(f"[Security] Dropped stale packet: SEQ {current_seq}")
            except Exception as e:
                print(f"[Ingestion_Error]: {e}")