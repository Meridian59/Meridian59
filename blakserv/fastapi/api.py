from fastapi import APIRouter, HTTPException
from maintenance import MaintenanceClient
import asyncio, re

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

@router.get("/admin/show-constant/{constant_name}")
async def show_constant(constant_name: str):
    """
    Show the value of a constant by its name.
    """
    try:
        # Send the command
        command = f"show constant {constant_name}"
        loop = asyncio.get_running_loop()
        response = await loop.run_in_executor(None, client.send_command, command)

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Parse the response
        lines = response.split('\r\n')
        if len(lines) < 2 or "There is no value for" in lines[1]:
            raise HTTPException(status_code=404, detail=f"Constant '{constant_name}' not found.")

        # Extract the constant value from the second line
        parts = lines[1].split("=", 1)
        if len(parts) != 2:
            raise HTTPException(status_code=500, detail="Unexpected response format.")

        constant_name = parts[0].strip()
        constant_value = parts[1].strip()

        return {
            "status": "success",
            "constant": {
                "name": constant_name,
                "value": constant_value
            }
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-list/{list_id}")
async def show_list(list_id: int):
    """
    Show the elements of a list by its ID.
    """
    try:
        # Send the command
        command = f"show list {list_id}"
        loop = asyncio.get_running_loop()
        response = await loop.run_in_executor(None, client.send_command, command)

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Parse the response
        lines = response.split('\r\n')
        if len(lines) < 3 or not lines[1].startswith(":<") or not lines[-1].startswith(":>"):
            raise HTTPException(status_code=404, detail=f"List '{list_id}' not found or invalid response format.")

        # Extract list elements
        list_elements = []
        for line in lines[2:-1]:  # Skip the first two lines (command echo and :<) and the last line (:>)
            line = line.strip()
            if line.startswith(":"):
                parts = line.split(None, 2)  # Split into type and value
                if len(parts) == 3 and parts[1] != "[" and parts[1] != "]":  # Skip structural markers
                    element_type = parts[1].strip(":")  # Remove the leading colon
                    element_value = parts[2].strip()
                    list_elements.append({
                        "type": element_type,
                        "value": element_value
                    })

        return {
            "status": "success",
            "list_id": list_id,
            "elements": list_elements
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-listnode/{list_id}")
async def show_listnode(list_id: int):
    """
    Show the details of a specific list node by its ID.
    """
    try:
        # Send the command
        command = f"show listnode {list_id}"
        loop = asyncio.get_running_loop()
        response = await loop.run_in_executor(None, client.send_command, command)

        print("Raw response:", repr(response))
        check_access(response)

        # Attempt to find 'first' and 'rest' with regex
        first_match = re.search(r"first\s*=\s*([A-Z$]+)\s+(.+)", response)
        rest_match = re.search(r"rest\s*=\s*([A-Z$]+)\s+(.+)", response)

        if not first_match or not rest_match:
            raise HTTPException(status_code=500, detail="Could not parse first or rest values.")

        # Parse 'first' and 'rest'
        first_parsed = {"type": first_match.group(1), "value": first_match.group(2).strip()}
        rest_parsed = {"type": rest_match.group(1), "value": rest_match.group(2).strip()}

        return {
            "status": "success",
            "list_id": list_id,
            "first": first_parsed,
            "rest": rest_parsed
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-message/{class_identifier}/{message_identifier}")
async def show_message(class_identifier: str, message_identifier: str):
    """
    Show details for a specific message by its class and message identifiers.
    
    Args:
        class_identifier (str): The class name or ID.
        message_identifier (str): The message name or ID.
    """
    try:
        # Send the command with the class and message identifiers
        command = f"show message {class_identifier} {message_identifier}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Parse the response
        lines = response.split('\r\n')
        parsed_data = {
            "class": None,
            "message": None,
            "parameters": [],
            "description": None
        }

        in_parameters_section = False
        for i, line in enumerate(lines):
            line = line.strip()
            if line.startswith("CLASS"):
                parsed_data["class"] = line.split(":", 1)[1].strip()
            elif line.startswith("MESSAGE"):
                parsed_data["message"] = line.split(":", 1)[1].strip()
            elif line.startswith("Parameters:"):
                in_parameters_section = True
            elif in_parameters_section:
                if line.startswith("----"):  # End of parameters section
                    in_parameters_section = False
                elif line:  # Parse parameter line
                    parts = line.split(None, 2)
                    if len(parts) == 3:
                        parsed_data["parameters"].append({
                            "name": parts[0],
                            "type": parts[1],
                            "value": parts[2]
                        })
            elif line.startswith("No description"):
                parsed_data["description"] = "No description"

        # Ensure all required fields are populated
        if not parsed_data["class"] or not parsed_data["message"]:
            raise HTTPException(status_code=500, detail="Failed to parse class or message details.")

        return {
            "status": "success",
            "class": parsed_data["class"],
            "message": parsed_data["message"],
            "parameters": parsed_data["parameters"],
            "description": parsed_data["description"]
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-name/{character_name}")
async def show_name(character_name: str):
    """
    Show the object ID of a specific character by their name.
    
    Args:
        character_name (str): The name of the character to query.
    """
    try:
        # Send the command
        command = f"show name {character_name}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the character does not exist
        if f"Cannot find user with name {character_name}" in response:
            raise HTTPException(status_code=404, detail=f"Character '{character_name}' not found.")

        # Parse the response
        lines = response.split('\r\n')
        object_id = None

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Parse the object ID from the line starting with ":< object"
            if line.startswith(":< object"):
                parts = line.split()
                if len(parts) == 3 and parts[1] == "object":
                    object_id = int(parts[2])
                    break

        # Ensure object_id is found
        if object_id is None:
            raise HTTPException(status_code=500, detail="Failed to parse object ID from response.")

        return {
            "status": "success",
            "character_name": character_name,
            "object_id": object_id
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-object/{object_id}")
async def show_object(object_id: int):
    """
    Show the details of a specific object by its ID.
    
    Args:
        object_id (int): The ID of the object to query.
    """
    try:
        # Send the command
        command = f"show object {object_id}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the object does not exist
        if f"Cannot find object {object_id}" in response:
            raise HTTPException(status_code=404, detail=f"Object {object_id} not found.")

        # Parse the response
        lines = response.split('\r\n')
        object_data = {
            "object_id": object_id,
            "class": None,
            "properties": {}
        }

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Handle combined lines (split on "\n:")
            if "\n:" in line:
                sub_lines = line.split("\n:")
                for sub_line in sub_lines:
                    sub_line = sub_line.strip()
                    if sub_line.startswith(":"):
                        sub_line = sub_line[1:]  # Remove leading ":"
                    parts = sub_line.split("=", 1)
                    if len(parts) == 2:
                        key = parts[0].strip()
                        value = parts[1].strip()
                        object_data["properties"][key] = value
                continue

            # Parse the object class
            if line.startswith(":< OBJECT"):
                parts = line.split()
                if len(parts) >= 5 and parts[3] == "is" and parts[4] == "CLASS":
                    object_data["class"] = parts[5]
                continue

            # Parse object properties
            if line.startswith(":"):
                parts = line.split("=", 1)
                if len(parts) == 2:
                    key = parts[0].strip(": ").strip()
                    value = parts[1].strip()
                    object_data["properties"][key] = value

        # Ensure the class is found
        if not object_data["class"]:
            raise HTTPException(status_code=500, detail="Failed to parse object class from response.")

        return {
            "status": "success",
            "object": object_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-string/{string_id}")
async def show_string(string_id: int):
    """
    Show the content of a specific string by its ID.
    
    Args:
        string_id (int): The ID of the string to query.
    """
    try:
        # Send the command
        command = f"show string {string_id}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the string does not exist
        if f"Cannot find string {string_id}" in response:
            raise HTTPException(status_code=404, detail=f"String {string_id} not found.")

        # Parse the response
        lines = response.split('\r\n')
        string_content = []
        in_string_section = False

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            if line.startswith("-------------------------------------------"):
                # Toggle the string section flag
                in_string_section = not in_string_section
                continue

            if in_string_section:
                string_content.append(line)

        # Join the string content, even if it's empty
        content = "\n".join(string_content).strip()

        return {
            "status": "success",
            "string_id": string_id,
            "content": content
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-usage")
async def show_usage():
    """
    Show the current server usage, including the number of active sessions.
    """
    try:
        # Send the command
        command = "show usage"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Parse the response
        lines = response.split('\r\n')
        usage_data = {}

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Parse the sessions line
            if line.startswith(":< sessions"):
                parts = line.split()
                if len(parts) == 3 and parts[1] == "sessions":
                    usage_data["sessions"] = int(parts[2])
                break

        # Ensure sessions data is found
        if "sessions" not in usage_data:
            raise HTTPException(status_code=500, detail="Failed to parse usage data from response.")

        return {
            "status": "success",
            "usage": usage_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-user/{username}")
async def show_user(username: str):
    """
    Show details for a specific user by their username.
    
    Args:
        username (str): The username to query.
    """
    try:
        # Send the command
        command = f"show user {username}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the user does not exist
        if f"Cannot find user {username}" in response:
            raise HTTPException(status_code=404, detail=f"User '{username}' not found.")

        # Parse the response
        lines = response.split('\r\n')
        user_data = {}

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Parse the user details
            if line.startswith("Acct"):
                # Skip the header line
                continue
            elif line:
                parts = line.split()
                if len(parts) >= 4:
                    user_data = {
                        "account": int(parts[0]),
                        "object_id": int(parts[1]),
                        "class": parts[2],
                        "name": parts[3]
                    }
                break

        # Ensure user data is found
        if not user_data:
            raise HTTPException(status_code=500, detail="Failed to parse user data from response.")

        return {
            "status": "success",
            "user": user_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-resource/{resource_identifier}")
async def show_resource(resource_identifier: str):
    """
    Show details for a specific resource by its ID or name.
    
    Args:
        resource_identifier (str): The ID (int) or name (string) of the resource to query.
    """
    try:
        # Send the command
        command = f"show resource {resource_identifier}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the resource does not exist
        if "There is no resource with id" in response or "There is no resource named" in response:
            raise HTTPException(status_code=404, detail=f"Resource '{resource_identifier}' not found.")

        # Parse the response
        lines = response.split('\r\n')
        resource_data = {}

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Parse the resource details
            if line.startswith("ID"):
                # Skip the header line
                continue
            elif line:
                parts = line.split(None, 3)  # Split into at most 4 parts
                if len(parts) == 4:
                    resource_data = {
                        "id": int(parts[0]),
                        "name": parts[1],
                        "value": parts[3]
                    }
                break

        # Ensure resource data is found
        if not resource_data:
            raise HTTPException(status_code=500, detail="Failed to parse resource data from response.")

        return {
            "status": "success",
            "resource": resource_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-table/{table_id}")
async def show_table(table_id: int):
    """
    Show details for a specific table by its ID.

    Args:
        table_id (int): The ID of the table to query.
    """
    try:
        # Send the command
        command = f"show table {table_id}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the table does not exist
        if f"Cannot find table {table_id}" in response:
            raise HTTPException(status_code=404, detail=f"Table {table_id} not found.")

        # Parse the response
        lines = response.split('\r\n')
        table_data = {
            "table_id": table_id,
            "size": None,
            "entries": []
        }

        # Extract table size
        size_match = re.search(rf"Table {table_id} \(size (\d+)\)", response)
        if size_match:
            table_data["size"] = int(size_match.group(1))

        # Ensure we have a valid table size
        if table_data["size"] is None:
            raise HTTPException(status_code=500, detail="Failed to parse table data from response.")

        # Regex pattern to match hash entries with both INT and RESOURCE keys
        entry_pattern = re.compile(r"hash\s+(\d+)\s*:\s*((?:\(key (INT|RESOURCE) \d+ val OBJECT \d+\)\s*)+)")

        for match in entry_pattern.finditer(response):
            hash_key = int(match.group(1))
            key_val_pairs = match.group(2)

            # Extract all (key TYPE <value> val OBJECT <value>) pairs
            pairs = re.findall(r"\(key (INT|RESOURCE) (\d+) val OBJECT (\d+)\)", key_val_pairs)

            for key_type, key_value, val_value in pairs:
                table_data["entries"].append({
                    "hash": hash_key,
                    "key_type": key_type,
                    "key": int(key_value),
                    "value": int(val_value)
                })

        return {
            "status": "success",
            "table": table_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/show-timer/{timer_id}")
async def show_timer(timer_id: int):
    """
    Show details for a specific timer by its ID.

    Args:
        timer_id (int): The ID of the timer to query.
    """
    try:
        # Send the command
        command = f"show timer {timer_id}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the timer does not exist
        if f"Cannot find timer {timer_id}" in response:
            raise HTTPException(status_code=404, detail=f"Timer {timer_id} not found.")

        # Parse the response
        lines = response.split('\r\n')
        timer_data = {}

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Parse the timer details
            parts = line.split()
            if len(parts) == 4 and parts[0].isdigit():
                timer_data = {
                    "timer_id": int(parts[0]),
                    "remaining_ms": int(parts[1]),
                    "object_id": int(parts[2]),
                    "message": parts[3]
                }
                break

        # Ensure timer data is found
        if not timer_data:
            raise HTTPException(status_code=500, detail="Failed to parse timer data from response.")

        return {
            "status": "success",
            "timer": timer_data
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/set-account-name/{account_id}")
async def set_account_name(account_id: int, new_name: str):
    """
    Set a new name for a specific account by its ID.

    Args:
        account_id (int): The ID of the account to update.
        new_name (str): The new name to set for the account.
    """
    try:
        # Send the command
        command = f"set account name {account_id} {new_name}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the account does not exist
        if f"Cannot find account {account_id}" in response:
            raise HTTPException(status_code=404, detail=f"Account {account_id} not found.")

        # Check if the response indicates success
        if f"Changing name of account {account_id}" not in response:
            raise HTTPException(status_code=500, detail="Failed to set account name. Unexpected response.")

        # Extract old and new account names from the response
        match = re.search(r"Changing name of account \d+ from '(.*?)' to '(.*?)'\.", response)
        if not match:
            raise HTTPException(status_code=500, detail="Failed to parse account name change response.")

        old_account_name, new_account_name = match.groups()

        return {
            "status": "success",
            "account_id": account_id,
            "old_account_name": old_account_name,
            "new_account_name": new_account_name
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))