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
    Get list of all online players.
    Returns structured JSON with player information, including objectID if present.
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
                            location = " ".join(parts[5:])
                            object_id = None

                            # Extract objectID from location if present
                            if "(" in location and ")" in location:
                                try:
                                    object_id = int(location.split("(")[-1].split(")")[0])
                                except ValueError:
                                    object_id = None

                            player = {
                                "name": parts[0],
                                "activity": parts[1] if parts[1] != "No" else None,
                                "version": parts[2] if parts[2] != "No" else None,
                                "session": parts[3],
                                "ip": parts[4],
                                "location": location,
                                "objectID": object_id
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

    Returns:
        JSON response with the object details.
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

        current_property = None

        for line in lines:
            line = line.strip()
            if not line or line.startswith(">"):  # Skip empty lines and command echo
                continue

            # Parse the object class
            if line.startswith(":< OBJECT"):
                parts = line.split()
                if len(parts) >= 5 and parts[3] == "is" and parts[4] == "CLASS":
                    object_data["class"] = parts[5]
                continue

            # Handle multi-line properties
            if "=" in line:
                # If there's a current property, save it
                if current_property:
                    key, value = current_property.split("=", 1)
                    object_data["properties"][key.strip(": ").strip()] = value.strip()

                # Start a new property
                current_property = line
            elif current_property:
                # Append to the current property if the line doesn't start a new one
                current_property += " " + line

        # Add the last property if it exists
        if current_property:
            key, value = current_property.split("=", 1)
            object_data["properties"][key.strip(": ").strip()] = value.strip()

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

@router.post("/admin/set-account-object/{account_id}/{object_id}")
async def set_account_object(account_id: int, object_id: int):
    """
    Associate an object with a specific account by its ID.

    Args:
        account_id (int): The ID of the account to update.
        object_id (int): The ID of the object to associate with the account.
    """
    try:
        # Send the command
        command = f"set account object {account_id} {object_id}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Handle specific error cases
        if f"Cannot find account {account_id}" in response:
            raise HTTPException(status_code=404, detail=f"Account {account_id} not found.")
        if f"Object {object_id} does not exist" in response:
            raise HTTPException(status_code=404, detail=f"Object {object_id} does not exist.")

        # Handle warnings
        warning_message = None
        if "warning:" in response.lower():
            warning_match = re.search(r"(warning:.*?expect\.)", response, re.IGNORECASE)
            if warning_match:
                warning_message = warning_match.group(1).strip()

        # Check if the response indicates success
        if f"Associated account {account_id} with object {object_id}" not in response:
            raise HTTPException(status_code=500, detail="Failed to set account object. Unexpected response.")

        # Extract old and new account names from the response
        old_account_name = None
        new_account_name = None
        match = re.search(r"Removing user object \d+ from the old account \d+.", response)
        if match:
            old_account_name = match.group(0).split("from the old account")[1].strip().strip(".")

        match_new = re.search(r"Associated account \d+ with object \d+ as a user.", response)
        if match_new:
            new_account_name = match_new.group(0).split("Associated account")[1].split("with object")[0].strip()

        return {
            "status": "success",
            "account_id": account_id,
            "object_id": object_id,
            "old_account_name": old_account_name,
            "new_account_name": new_account_name,
            "warning_message": warning_message
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/set-account-password/{account_id}")
async def set_account_password(account_id: int, new_password: str):
    """
    Set a new password for a specific account by its ID.

    Args:
        account_id (int): The ID of the account to update.
        new_password (str): The new password to set for the account.
    """
    try:
        # Send the command
        command = f"set account password {account_id} {new_password}"
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
        match = re.search(r"Set password for account (\d+) \((.*?)\)\.", response)
        if not match:
            raise HTTPException(status_code=500, detail="Failed to set account password. Unexpected response.")

        account_id = int(match.group(1))
        account_name = match.group(2)

        return {
            "status": "success",
            "account_id": account_id,
            "account_name": account_name
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/set-class")
async def set_class(class_name: str, var_name: str, value_type: str, value: str):
    """
    Set a class variable for a specific class.

    Args:
        class_name (str): The name of the class to modify.
        var_name (str): The name of the class variable to set.
        value_type (str): The type of the value (e.g., "int", "string").
        value (str): The value to set for the class variable.
    """
    try:
        # Send the command
        command = f"set class {class_name} {var_name} {value_type} {value}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Handle specific error cases
        if f"Cannot find class named {class_name}" in response:
            raise HTTPException(status_code=404, detail=f"Class '{class_name}' not found.")
        if f"Cannot find classvar named {var_name}" in response:
            raise HTTPException(status_code=404, detail=f"Class variable '{var_name}' not found in class '{class_name}'.")
        if f"'{value_type}' is not a tag." in response:
            raise HTTPException(status_code=400, detail=f"Invalid value type '{value_type}'.")
        if f"'{value}' is not valid data." in response:
            raise HTTPException(status_code=400, detail=f"Invalid value '{value}' for type '{value_type}'.")

        # Check if the response indicates success
        if f"> set class {class_name} {var_name} {value_type} {value}" not in response:
            raise HTTPException(status_code=500, detail="Failed to set class variable. Unexpected response.")

        return {
            "status": "success",
            "class_name": class_name,
            "var_name": var_name,
            "value_type": value_type,
            "value": value
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/set-object")
async def set_object(object_id: int, property_name: str, value_type: str, value: str):
    """
    Set a property of a specific object.

    Args:
        object_id (int): The ID of the object to modify.
        property_name (str): The name of the property to set.
        value_type (str): The type of the value (e.g., "int", "string").
        value (str): The value to set for the property.
    """
    try:
        # Send the command
        command = f"set object {object_id} {property_name} {value_type} {value}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Handle specific error cases
        if "Invalid object id" in response:
            raise HTTPException(status_code=404, detail=f"Invalid object ID {object_id} (or it has been deleted).")

        if "Property" in response and "doesn't exist" in response:
            # Extract the class name and ID from the response
            match = re.search(r"Property .* doesn't exist \(at least for CLASS (\w+) \((\d+)\)\)", response)
            if match:
                target_class_name = match.group(1)
                target_class_id = match.group(2)
                raise HTTPException(
                    status_code=404,
                    detail=f"Property '{property_name}' doesn't exist (at least for CLASS {target_class_name} ({target_class_id}))."
                )
            else:
                raise HTTPException(
                    status_code=404,
                    detail=f"Property '{property_name}' doesn't exist (at least for the object's class)."
                )

        if f"'{value_type}' is not a tag." in response:
            raise HTTPException(status_code=400, detail=f"Invalid value type '{value_type}'.")
        if f"'{value}' is not valid data." in response:
            raise HTTPException(status_code=400, detail=f"Invalid value '{value}' for type '{value_type}'.")

        # Check if the response indicates success
        if f"> set object {object_id} {property_name} {value_type} {value}" not in response:
            raise HTTPException(status_code=500, detail="Failed to set object property. Unexpected response.")

        return {
            "status": "success",
            "object_id": object_id,
            "property_name": property_name,
            "value_type": value_type,
            "value": value
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/set-config-boolean")
async def set_config_boolean(group: str, name: str, value: str):
    """
    Set a boolean configuration option.

    Args:
        group (str): The configuration group (e.g., Socket).
        name (str): The name of the configuration option.
        value (str): The value to set (e.g., 'Yes' or 'No').
    """
    try:
        # Send the command
        command = f"set config boolean {group} {name} {value}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Missing parameter" in response:
            raise HTTPException(status_code=400, detail="Missing required parameters.")
        if "Boolean configuration options must be 'yes' or 'no'" in response:
            raise HTTPException(status_code=400, detail="Boolean configuration options must be 'Yes' or 'No'.")
        if "Unable to find configure group" in response:
            raise HTTPException(status_code=404, detail=f"Unable to find configure group {group} name {name}.")
        if "This configure option is not a boolean" in response:
            raise HTTPException(status_code=400, detail=f"Configuration option {group} {name} is not a boolean.")

        # Check if the response indicates success
        if f"Configure option group {group} name {name} is now set to" not in response:
            raise HTTPException(status_code=500, detail="Failed to set boolean configuration. Unexpected response.")

        return {
            "status": "success",
            "group": group,
            "name": name,
            "value": value
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/set-config-integer")
async def set_config_integer(group: str, name: str, value: int):
    """
    Set an integer configuration option.

    Args:
        group (str): The configuration group (e.g., Login).
        name (str): The name of the configuration option.
        value (int): The value to set.
    """
    try:
        # Send the command
        command = f"set config int {group} {name} {value}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Missing parameter" in response:
            raise HTTPException(status_code=400, detail="Missing required parameters.")
        if "Unable to find configure group" in response:
            raise HTTPException(status_code=404, detail=f"Unable to find configure group {group} name {name}.")
        if "This configure option is not an integer" in response:
            raise HTTPException(status_code=400, detail=f"Configuration option {group} {name} is not an integer.")

        # Check if the response indicates success
        if f"Configure option group {group} name {name} is now set to" not in response:
            raise HTTPException(status_code=500, detail="Failed to set integer configuration. Unexpected response.")

        return {
            "status": "success",
            "group": group,
            "name": name,
            "value": value
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/set-config-string")
async def set_config_string(group: str, name: str, value: str):
    """
    Set a string configuration option.

    Args:
        group (str): The configuration group (e.g., Email).
        name (str): The name of the configuration option.
        value (str): The value to set.
    """
    try:
        # Send the command
        command = f"set config string {group} {name} {value}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Missing parameter" in response:
            raise HTTPException(status_code=400, detail="Missing required parameters.")
        if "Unable to find configure group" in response:
            raise HTTPException(status_code=404, detail=f"Unable to find configure group {group} name {name}.")
        if "This configure option is not a string" in response:
            raise HTTPException(status_code=400, detail=f"Configuration option {group} {name} is not a string.")

        # Check if the response indicates success
        if f"Configure option group {group} name {name} is now set to" not in response:
            raise HTTPException(status_code=500, detail="Failed to set string configuration. Unexpected response.")

        return {
            "status": "success",
            "group": group,
            "name": name,
            "value": value
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/suspend-account")
async def suspend_account(hours: int, account_identifier: str):
    """
    Suspend an account for a specified number of hours.

    Args:
        hours (int): The number of hours to suspend the account.
        account_identifier (str): The account name or ID to suspend.
    """
    try:
        # Send the command
        command = f"suspend account {hours} {account_identifier}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Missing parameter" in response:
            raise HTTPException(status_code=400, detail="Missing required parameters.")
        if "Cannot find account" in response:
            raise HTTPException(status_code=404, detail=f"Account '{account_identifier}' not found.")
        if "Suspension of account" in response and "failed" in response:
            raise HTTPException(status_code=500, detail=f"Failed to suspend account '{account_identifier}'.")

        # Check if the response indicates success
        match = re.search(r"Account (\d+) \((.*?)\) is suspended until (.+)\.", response)
        if not match:
            raise HTTPException(status_code=500, detail="Unexpected response format.")

        # Extract account details from the response
        account_id = match.group(1)
        account_name = match.group(2)
        suspended_until = match.group(3)

        return {
            "status": "success",
            "account_identifier": account_identifier,
            "account_id": account_id,
            "account_name": account_name,
            "hours_added": hours,
            "suspended_until": suspended_until
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/unsuspend-account")
async def unsuspend_account(account_identifier: str):
    """
    Unsuspend an account immediately.

    Args:
        account_identifier (str): The account name or ID to unsuspend.
    """
    try:
        # Send the command
        command = f"unsuspend account {account_identifier}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Cannot find account" in response:
            raise HTTPException(status_code=404, detail=f"Account '{account_identifier}' not found.")
        if "is unsuspended" not in response:
            raise HTTPException(status_code=500, detail="Unexpected response format.")

        # Extract account details from the response
        match = re.search(r"Account (\d+) \((.*?)\) is unsuspended\.", response)
        if not match:
            raise HTTPException(status_code=500, detail="Failed to parse unsuspend response.")

        account_id = match.group(1)
        account_name = match.group(2)

        return {
            "status": "success",
            "account_identifier": account_identifier,
            "account_id": account_id,
            "account_name": account_name
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/suspend-user")
async def suspend_user(hours: int, username: str):
    """
    Suspend a user for a specified number of hours.

    Args:
        hours (int): The number of hours to suspend the user.
        username (str): The name of the user to suspend.
    """
    try:
        # Send the command
        command = f"suspend user {hours} {username}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Missing parameter" in response:
            raise HTTPException(status_code=400, detail="Missing required parameters.")
        if "Cannot find user" in response:
            raise HTTPException(status_code=404, detail=f"User '{username}' not found.")
        if "Suspension of account" in response and "failed" in response:
            raise HTTPException(status_code=500, detail=f"Failed to suspend user '{username}'.")

        # Check if the response indicates success
        match = re.search(r"Account (\d+) \((.*?)\) is suspended until (.+)\.", response)
        if not match:
            raise HTTPException(status_code=500, detail="Unexpected response format.")

        # Extract account details from the response
        account_id = match.group(1)
        account_name = match.group(2)
        suspended_until = match.group(3)

        return {
            "status": "success",
            "username": username,
            "account_id": account_id,
            "account_name": account_name,
            "hours_added": hours,
            "suspended_until": suspended_until
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/unsuspend-user")
async def unsuspend_user(username: str):
    """
    Unsuspend a user immediately.

    Args:
        username (str): The name of the user to unsuspend.
    """
    try:
        # Send the command
        command = f"unsuspend user {username}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Cannot find user" in response:
            raise HTTPException(status_code=404, detail=f"User '{username}' not found.")
        if "is unsuspended" not in response:
            raise HTTPException(status_code=500, detail="Unexpected response format.")

        # Extract account details from the response
        match = re.search(r"Account (\d+) \((.*?)\) is unsuspended\.", response)
        if not match:
            raise HTTPException(status_code=500, detail="Failed to parse unsuspend response.")

        account_id = match.group(1)
        account_name = match.group(2)

        return {
            "status": "success",
            "username": username,
            "account_id": account_id,
            "account_name": account_name
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/send-object")
async def send_object(
    object_id: int,
    message: str,
    param1: str = None,
    param2: str = None,
    param3: str = None,
    param4: str = None,
    param5: str = None,
    param6: str = None,
    param7: str = None,
    param8: str = None
):
    """
    Send a message to a specific object.

    Args:
        object_id (int): The ID of the object to send the message to.
        message (str): The message to send to the object.
        param1 to param8 (str, optional): Optional parameters.
    """
    try:
        # Build the command
        command = f"send object {object_id} {message}"
        for param in [param1, param2, param3, param4, param5, param6, param7, param8]:
            if param:
                command += f" {param}"

        # Send the command
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Invalid object id" in response:
            raise HTTPException(status_code=404, detail=f"Object ID {object_id} not found or invalid.")
        if "Unknown message" in response:
            raise HTTPException(status_code=400, detail=f"Unknown message '{message}' for object {object_id}.")
        if "Invalid parameter" in response:
            raise HTTPException(status_code=400, detail="Invalid parameter provided.")

        # Check if the response includes the expected return message
        match = re.search(r":< return from OBJECT \d+ MESSAGE .*?\r\n: (.*?)\r\n:>", response, re.DOTALL)
        return_value = match.group(1).strip() if match else None

        return {
            "status": "success",
            "object_id": object_id,
            "message": message,
            "parameters": [param for param in [param1, param2, param3, param4, param5, param6, param7, param8] if param],
            "return": return_value,
            "raw_response": response.strip()  # Include the raw response for debugging
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/send-class")
async def send_class(
    class_name: str,
    message: str,
    param1: str = None,
    param2: str = None,
    param3: str = None,
    param4: str = None,
    param5: str = None,
    param6: str = None,
    param7: str = None,
    param8: str = None
):
    """
    Send a message to all instances of a specific class.

    Args:
        class_name (str): The name of the class to send the message to.
        message (str): The message to send to the class instances.
        param1 to param8 (str, optional): Optional parameters.
    """
    try:
        # Build the command
        command = f"send class {class_name} {message}"
        for param in [param1, param2, param3, param4, param5, param6, param7, param8]:
            if param:
                command += f" {param}"

        # Send the command
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Invalid class name" in response:
            raise HTTPException(status_code=404, detail=f"Class '{class_name}' not found or invalid.")
        if "Unknown message" in response:
            raise HTTPException(status_code=400, detail=f"Unknown message '{message}' for class '{class_name}'.")
        if "Invalid parameter" in response:
            raise HTTPException(status_code=400, detail="Invalid parameter provided.")

        # Check if the response includes the expected return message
        match = re.search(r":< (\d+) instance\(s\) sent MESSAGE \d+ .*?\r\n:>", response, re.DOTALL)
        instances_sent = int(match.group(1)) if match else None

        return {
            "status": "success",
            "class_name": class_name,
            "message": message,
            "parameters": [param for param in [param1, param2, param3, param4, param5, param6, param7, param8] if param],
            "instances_sent": instances_sent,
            "raw_response": response.strip()  # Include the raw response for debugging
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/save-game")
async def save_game():
    """
    Save the game state, performing garbage collection first.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "save game"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Garbage collecting and saving game" not in response:
            raise HTTPException(status_code=500, detail="Failed to save game. Unexpected response.")

        # Extract save time if available
        match = re.search(r"Save time is \((\d+)\)", response)
        save_time = int(match.group(1)) if match else None

        return {
            "status": "success",
            "message": "Game saved successfully.",
            "save_time": save_time
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/save-configuration")
async def save_configuration():
    """
    Save the server configuration to the configuration file.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "save configuration"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Handle specific error cases
        if "Configuration saved." not in response:
            raise HTTPException(status_code=500, detail="Failed to save configuration. Unexpected response.")

        return {
            "status": "success",
            "message": "Configuration saved successfully."
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/terminate-save")
async def terminate_save():
    """
    Terminate the server after saving the game state.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "terminate save"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the response indicates saving and termination
        save_complete = "Garbage collecting and saving game... done." in response
        termination_confirmed = "Terminating server. All connections, including yours, about to be lost" in response

        # If the expected output is received, return success
        if save_complete and termination_confirmed:
            return {
                "status": "success",
                "message": "Server termination initiated after saving.",
                "save_complete": save_complete,
                "termination_confirmed": termination_confirmed,
                "raw_response": response.strip()
            }

        # If the expected output is not received, raise an error
        raise HTTPException(status_code=500, detail="Failed to terminate server with save. Unexpected response.")

    except Exception as e:
        # Handle the case where the connection is forcibly closed
        if "forcibly closed by the remote host" in str(e):
            return {
                "status": "success",
                "message": "Server termination initiated after saving. Connection was forcibly closed by the server.",
                "save_complete": True,
                "termination_confirmed": True,
                "raw_response": response.strip() if 'response' in locals() else None
            }

        # Raise other exceptions as usual
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/terminate-nosave")
async def terminate_nosave():
    """
    Terminate the server without saving the game state.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "terminate nosave"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Since terminate nosave does not provide output, assume success if no exception occurs
        return {
            "status": "success",
            "message": "Server termination initiated without saving.",
            "raw_response": response.strip() if response else None
        }

    except Exception as e:
        # Handle the case where the connection is forcibly closed
        if "forcibly closed by the remote host" in str(e):
            return {
                "status": "success",
                "message": "Server termination initiated without saving. Connection was forcibly closed by the server.",
                "raw_response": None
            }

        # Raise other exceptions as usual
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/mark")
async def mark():
    """
    Add a marker to the server logs.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "mark"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Since there is no output for the `mark` command, assume success if no exception occurs
        return {
            "status": "success",
            "message": "Marker added to the server logs.",
            "raw_response": response.strip() if response else None
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/lock")
async def lock(reason: str = "The game is temporarily closed for maintenance."):
    """
    Lock the game, preventing new players from joining.

    Args:
        reason (str, optional): The reason for locking the game. Defaults to a maintenance message.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = f"lock {reason}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the response indicates success
        if f"Locking game <{reason}>" not in response:
            raise HTTPException(status_code=500, detail="Failed to lock the game. Unexpected response.")

        return {
            "status": "success",
            "message": f"Game locked successfully with reason: {reason}",
            "raw_response": response.strip()
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/unlock")
async def unlock():
    """
    Unlock the game, allowing new players to join.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "unlock"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the response indicates success
        if "Unlocking game." not in response:
            raise HTTPException(status_code=500, detail="Failed to unlock the game. Unexpected response.")

        return {
            "status": "success",
            "message": "Game unlocked successfully.",
            "raw_response": response.strip()
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/reload-game")
async def reload_game(save_time: int = 0):
    """
    Reload the game from a specific save time.

    Args:
        save_time (int, optional): The save time to reload from. Defaults to 0 (last save).

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = f"reload game {save_time}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the response includes expected output
        if "Unloading game... done." in response and "Loading game... done." in response:
            return {
                "status": "success",
                "message": f"Game reloaded successfully from save time {save_time}.",
                "save_time": save_time,
                "raw_response": response.strip()
            }

        # If no reliable output, assume success
        return {
            "status": "success",
            "message": "Command sent successfully.",
            "save_time": save_time,
            "raw_response": response.strip()
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/reload-motd")
async def reload_motd():
    """
    Reload the message of the day from the file.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "reload motd"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the response indicates success
        if "Reloading motd..." in response and "done." in response:
            return {
                "status": "success",
                "message": "MOTD reloaded successfully.",
                "raw_response": response.strip()
            }

        # If no reliable output, assume success
        return {
            "status": "success",
            "message": "Command sent successfully.",
            "raw_response": response.strip()
        }

    except Exception as e:
        # Handle the case where the connection is forcibly closed or other issues
        if "forcibly closed by the remote host" in str(e):
            return {
                "status": "success",
                "message": "MOTD reloaded successfully. Connection was forcibly closed by the server.",
                "raw_response": None
            }

        # Raise other exceptions as usual
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/reload-packages")
async def reload_packages():
    """
    Rescan the upload directory for packages.

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the command
        command = "reload packages"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the response indicates success
        if "Reloading packages... done." not in response:
            raise HTTPException(status_code=500, detail="Failed to reload packages. Unexpected response.")

        return {
            "status": "success",
            "message": "Packages reloaded successfully.",
            "raw_response": response.strip()
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/reload-system")
async def reload_system():
    """
    Save the game and reload all kod, motd, and other system components.

    Returns:
        JSON response indicating success or failure for each step.
    """
    try:
        # Send the command
        command = "reload system"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Normalize the response by removing extra whitespace and combining lines
        normalized_response = " ".join(response.split())

        # Parse the response for specific steps
        garbage_collection_success = "Garbage collecting and saving game... done." in normalized_response
        unloading_success = "Unloading game, kodbase, and .bof ... done." in normalized_response
        loading_success = "Loading game, kodbase, and .bof ... done." in normalized_response

        # Return structured response
        return {
            "status": "success",
            "steps": {
                "garbage_collection": "success" if garbage_collection_success else "failed",
                "saving_game": "success" if garbage_collection_success else "failed",
                "reloading_components": "success" if unloading_success and loading_success else "failed"
            },
            "raw_response": response.strip()
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/admin/read/{filename}")
async def admin_read(filename: str):
    """
    Read the contents of a file on the server, execute each line as a command,
    and capture the output for each command.

    Args:
        filename (str): The name of the file to read.

    Returns:
        JSON response with the file contents and the output for each command.
    """
    try:
        # Send the command to read the file
        command = f"read {filename}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if the file could not be opened
        if f"Error opening {filename}" in response:
            raise HTTPException(status_code=404, detail=f"File '{filename}' not found or cannot be opened.")

        # Parse the response to extract commands and their outputs
        lines = response.split('\r\n')
        command_outputs = []
        current_command = None

        for line in lines:
            line = line.strip()
            if not line:
                continue

            # Detect a command line (starts with ">>")
            if line.startswith(">>"):
                # If there's a previous command, add it to the results
                if current_command:
                    command_outputs.append(current_command)

                # Start a new command
                current_command = {
                    "command": line[2:].strip(),  # Remove ">>" and trim
                    "response": ""
                }
            elif current_command:
                # Append the line to the current command's response
                current_command["response"] += line + "\n"

        # Add the last command if it exists
        if current_command:
            command_outputs.append(current_command)

        # Clean up responses (strip trailing newlines)
        for cmd in command_outputs:
            cmd["response"] = cmd["response"].strip()

        # Check for access denial on a per-command basis
        for cmd in command_outputs:
            if "You do not have access to this command." in cmd["response"]:
                cmd["error"] = "Access denied for this command."

        return {
            "status": "success",
            "filename": filename,
            "commands": command_outputs
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/admin/say")
async def admin_say(message: str):
    """
    Send a message to all logged in admins.  Note: shows up only in admin mode (not in game text).

    Args:
        message (str): The message to send to .

    Returns:
        JSON response indicating success or failure.
    """
    try:
        # Send the adminsay command with the message
        command = f"say {message}"
        response = await asyncio.get_event_loop().run_in_executor(
            None, client.send_command, command
        )

        # Debug the raw response
        print("Raw response:", repr(response))

        # Check if access is denied
        check_access(response)

        # Check if the response indicates success
        if "Said." not in response:
            raise HTTPException(status_code=500, detail="Failed to send message. Unexpected response.")

        return {
            "status": "success",
            "message": f"Sent message: {message}",
            "raw_response": response.strip()
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))