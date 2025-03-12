import socket
import logging
from typing import Optional

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

class MaintenanceClient:
    def __init__(self, host: str = "127.0.0.1", port: int = 9998):
        self.host = host
        self.port = port
        self._socket: Optional[socket.socket] = None

    def send_command(self, command: str) -> str:
        try:
            # Create raw socket
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._socket.settimeout(1)  # 1 second timeout
            
            # Connect
            self._socket.connect((self.host, self.port))
            logger.debug(f"Connected to {self.host}:{self.port}")
            
            # Send command once
            full_command = f"{command}\r\n".encode()
            self._socket.send(full_command)
            logger.debug(f"Sent command: {command!r}")
            
            # Try to read response
            response_lines = []
            try:
                while True:
                    data = self._socket.recv(1024)
                    if not data:
                        break
                    decoded = data.decode().strip()
                    logger.debug(f"Received: {decoded!r}")
                    if decoded and "Disconnecting for idleness" not in decoded:
                        response_lines.append(decoded)
            except socket.timeout:
                logger.debug("Read timeout - expected")
            
            return "\n".join(filter(None, response_lines))

        except Exception as e:
            logger.error(f"Error: {str(e)}")
            raise
        finally:
            if self._socket:
                self._socket.close()