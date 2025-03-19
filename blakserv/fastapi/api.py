from fastapi import APIRouter, HTTPException
from maintenance import MaintenanceClient
import asyncio

router = APIRouter()
client = MaintenanceClient()

def check_access(response: str):
    """
    Check if the server denies access to the command.
    Raises an HTTPException if access is denied.
    """
    if "You do not have access to this command." in response:
        raise HTTPException(
            status_code=403,
            detail="You do not have access to this command."
        )

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
        
        check_access(response)

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

        check_access(response)
        
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

@router.get("/admin/show-clock")
async def show_clock():
    """
    Show clock information
    """
    try:
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, "show clock"
        )

        check_access(response)

        # Parse the response
        if "Current server clock reads" in response:
            parts = response.split(" ")
            try:
                # Find the part that contains the epoch time
                epoch_time_index = parts.index("reads") + 1
                epoch_time = int(parts[epoch_time_index])
                date_time = " ".join(parts[epoch_time_index + 1:]).strip("().")
                return {
                    "status": "success",
                    "epoch_time": epoch_time,
                    "date_time": date_time
                }
            except (ValueError, IndexError) as e:
                raise HTTPException(status_code=500, detail="Error parsing response: " + str(e))
        else:
            raise HTTPException(status_code=500, detail="Unexpected response format")
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-accounts")
async def show_accounts():
    """
    Show all accounts
    """
    try:
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, "show accounts"
        )

        check_access(response)

        # Parse the response
        lines = response.split('\r\n')
        accounts = []
        
        for line in lines:
            parts = line.split()
            if len(parts) >= 6:
                account = {
                    "account": parts[0],
                    "name": parts[1],
                    "suspended": parts[2] if parts[2] != "" else None,
                    "credits": float(parts[3]),
                    "last_login": " ".join(parts[4:])
                }
                accounts.append(account)
        
        return {
            "status": "success",
            "accounts": accounts
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-account/{account}")
async def show_account(account: str):
    """
    Show details for a specific account by name or number.
    """
    try:
        # Send the command with the account parameter
        command = f"show account {account}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the account does not exist
        if f"Cannot find account {account}" in response:
            raise HTTPException(status_code=404, detail=f"Account {account} not found.")

        # Parse the response
        lines = response.split('\r\n')
        account_data = {}
        user_data = []

        # Parse account details
        parsing_users = False
        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            if line.startswith("Acct") and "Object" in line:  # Start of user details
                parsing_users = True
                continue

            if line.startswith("Acct") and "Credits" in line:  # Skip account header
                continue

            if not parsing_users:  # Parse account summary
                # Extract fields based on spaces
                parts = line.split(maxsplit=4)  # Split into at most 5 parts
                if len(parts) >= 5:
                    account = parts[0]  # First field: account
                    name = parts[1]  # Second field: name
                    credits = float(parts[2])  # Third field: credits
                    last_login = " ".join(parts[3:])  # Remaining fields: last_login

                    account_data = {
                        "account": account,
                        "name": name,
                        "credits": credits,
                        "last_login": last_login
                    }
            elif parsing_users:  # Parse user details
                parts = line.split(maxsplit=4)  # Split into at most 4 parts
                if len(parts) == 4:
                    user_data.append({
                        "account": parts[0],
                        "object": parts[1],
                        "class": parts[2],
                        "name": parts[3]
                    })

        # Ensure account_data is not empty
        if not account_data:
            raise HTTPException(status_code=404, detail="Account not found or invalid response format.")

        return {
            "status": "success",
            "account": account_data,
            "users": user_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-belong/{object_id}")
async def show_belong(object_id: int):
    """
    Show objects owned by a specific object ID.
    """
    try:
        # Send the command with the object ID parameter
        command = f"show belong {object_id}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the object does not exist
        if f"{object_id} owns the following objects -" not in response:
            raise HTTPException(status_code=404, detail=f"Object {object_id} not found or owns no objects.")

        # Parse the response
        lines = response.split('\r\n')
        owner = None
        objects = []
        current_object = None

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Identify the owner object
            if line.endswith("owns the following objects -"):
                owner = int(line.split()[0])
                continue

            # Start of a new object block
            if line.startswith(":< OBJECT"):
                if current_object:  # Save the previous object
                    objects.append(current_object)
                parts = line.split()
                current_object = {
                    "object_id": int(parts[2]),
                    "class": parts[-1],
                    "properties": {}
                }
                continue

            # End of an object block
            if line.startswith(":>"):
                if current_object:
                    objects.append(current_object)
                    current_object = None
                continue

            # Parse object properties
            if current_object and line.startswith(":"):
                parts = line.split("=", 1)
                key = parts[0].strip(": ").strip()
                value = parts[1].strip() if len(parts) > 1 else None
                current_object["properties"][key] = value

        # Ensure the last object is added
        if current_object:
            objects.append(current_object)

        # Return the parsed data
        return {
            "status": "success",
            "owner": owner,
            "objects": objects
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-class/{name}")
async def show_class(name: str):
    """
    Show details for a specific class by name.
    """
    try:
        # Normalize the class name to match the expected case
        normalized_name = name.capitalize()

        # Send the command with the normalized class name
        command = f"show class {normalized_name}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the class does not exist
        if f"CLASS {normalized_name}" not in response:
            raise HTTPException(status_code=404, detail=f"Class {name} not found.")

        # Parse the response
        lines = response.split('\r\n')
        class_data = {
            "name": None,
            "id": None,
            "file": None,
            "variables": [],
            "messages": []
        }

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Parse the class header
            if line.startswith(":< CLASS"):
                parts = line.split()
                class_data["name"] = parts[2]
                class_data["id"] = int(parts[3].strip("()"))
                class_data["file"] = parts[4]
                continue

            # Parse variables
            if line.startswith(": VAR"):
                parts = line.split("=", 1)
                variable_name = parts[0].strip(": VAR").strip()
                variable_value = parts[1].strip() if len(parts) > 1 else None
                class_data["variables"].append({
                    "name": variable_name,
                    "value": variable_value
                })
                continue

            # Parse messages
            if line.startswith(": MSG"):
                message_name = line.strip(": MSG").strip()
                class_data["messages"].append(message_name)
                continue

        return {
            "status": "success",
            "class": class_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-configuration")
async def show_configuration():
    """
    Show the server configuration.
    """
    try:
        command = "show configuration"
        loop = asyncio.get_running_loop()
        response = await loop.run_in_executor(None, client.send_command, command)

        print("Raw response:", repr(response))

        check_access(response)

        lines = response.split('\r\n')
        configuration = {}
        current_section = None
        current_key = None

        for line in lines:
            line = line.strip()
            print("Processing line:", repr(line))

            if not line or line.startswith(">"):
                continue

            if line.startswith("[") and line.endswith("]"):
                current_section = line.strip("[]")
                configuration[current_section] = {}
                current_key = None
                print(f"New section: {current_section}")
                continue

            if current_section:
                parts = line.split(None, 1)  # Split by whitespace once
                if len(parts) == 2:
                    key, value = parts
                    configuration[current_section][key.strip()] = value.strip()
                    current_key = key.strip()
                    print(f"Added key-value pair: {key} = {value}")
                elif current_key:
                    configuration[current_section][current_key] += f" {line.strip()}"
                    print(f"Appended to key {current_key}: {line.strip()}")

        print("Final configuration:", configuration)

        return {
            "status": "success",
            "configuration": configuration
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
