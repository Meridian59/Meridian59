from fastapi import APIRouter, HTTPException
from maintenance import MaintenanceClient
import asyncio

router = APIRouter()
client = MaintenanceClient()

@router.get("/admin/who")
async def get_online_players():
    """
    Get list of all online players
    Returns structured JSON with player information
    """
    try:
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, "who"
        )
        
        # Parse the response
        lines = response.split('\r\n')
        players = []
        
        # Find the header line
        for i, line in enumerate(lines):
            if 'Name' in line and 'Act' in line and 'Ver' in line:
                # Next line is separator, actual data starts after
                headers = [h.strip() for h in line.split() if h.strip()]
                
                # Process each player line
                for player_line in lines[i+2:]:  # Skip header and separator
                    if player_line and not player_line.startswith('>') and not player_line.startswith('--'):
                        # Split on multiple spaces
                        parts = [p for p in player_line.split() if p.strip()]
                        if len(parts) >= 6:  # Ensure we have enough parts
                            player = {
                                "name": parts[0],
                                "activity": parts[1] if parts[1] != "No" else None,
                                "version": parts[2] if parts[2] != "No" else None,
                                "session": parts[3],
                                "ip": parts[4],
                                "location": " ".join(parts[5:])
                            }
                            players.append(player)
                break
        
        return {
            "status": "success",
            "players": players,
            "total_players": len(players)
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/send-users")
async def send_users_message(message: str):
    """
    Send message to all logged in users
    
    Args:
        message (str): The message to send to all users
    """
    try:
        command = f'send users "{message}"'
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )
        return {"status": "success", "response": response or "Message sent"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/status")
async def get_server_status():
    """
    Get server status information
    Returns structured JSON with server statistics
    """
    try:
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, "show status"
        )
        
        # Parse the response
        lines = response.split('\r\n')
        status_data = {}
        
        for line in lines:
            line = line.strip()
            
            # Skip empty lines and separators
            if not line or line.startswith('>') or line.startswith('---'):
                continue
                
            # Parse version and build info
            if "BlakSton Server" in line:
                parts = line.split('(')
                status_data["version"] = parts[0].replace("BlakSton Server", "").strip()
                status_data["build_date"] = parts[1].replace(")", "").strip()
                
            # Parse current time
            elif "Current time is" in line:
                status_data["current_time"] = line.replace("Current time is", "").strip()
                
            # Parse uptime
            elif "System started at" in line:
                parts = line.split('(')
                status_data["start_time"] = parts[0].replace("System started at", "").strip()
                status_data["uptime"] = parts[1].replace(")", "").strip()
                
            # Parse performance metrics
            elif "Interpreted" in line:
                status_data["instructions"] = {
                    "total": line.split()[1],
                    "period": line.split()[-2]
                }
                
            # Parse message stats
            elif "Handled" in line:
                parts = line.split()
                status_data["messages"] = {
                    "top_level": parts[1],
                    "total": parts[-2]
                }
                
            # Parse account info
            elif "Active accounts:" in line:
                status_data["accounts"] = {
                    "active": line.split(":")[1].strip()
                }
                
            # Parse session info
            elif "sessions logged on" in line:
                status_data["sessions"] = {
                    "count": line.split()[2]
                }
                
            # Parse resource usage
            elif "Used" in line and "nodes" in line:
                parts = line.split()
                node_type = parts[-1].replace("nodes", "").strip()
                status_data.setdefault("resources", {})[node_type] = parts[1]
                
            # Parse timer info
            elif "Watching" in line and "timers" in line:
                status_data["active_timers"] = line.split()[1]

        return {
            "status": "success",
            "server_status": status_data
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/memory")
async def get_memory_usage():
    """Get server memory usage information"""
    try:
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, "show memory"
        )
        
        lines = [line.strip() for line in response.split('\r\n')]
        memory_data = {
            "timestamp": "",
            "components": {},
            "total_mb": 0
        }
        
        # Skip header section
        start_index = next((i for i, line in enumerate(lines) 
                          if "System Memory" in line), 0)
        
        for line in lines[start_index:]:
            # Skip empty lines, prompts, headers and separators
            if (not line or 
                line.startswith('>') or 
                '-' in line or       # Skip any line with dashes
                "System Memory" in line):
                continue
            
            # Get timestamp
            if "2025" in line:
                memory_data["timestamp"] = line
                continue
            
            # Get total memory
            if "Total" in line:
                parts = line.split()
                try:
                    # Look for the number before "MB"
                    mb_index = parts.index("MB") - 1
                    if mb_index > 0:
                        memory_data["total_mb"] = int(parts[mb_index])
                except (ValueError, IndexError):
                    continue
                continue
            
            # Parse component lines - must have exactly 2 parts
            parts = line.split()
            if len(parts) == 2:
                component = parts[0].lower()
                try:
                    value = int(parts[1].replace(',', ''))
                    memory_data["components"][component] = value
                except ValueError:
                    continue

        return {
            "status": "success",
            "memory": memory_data
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))