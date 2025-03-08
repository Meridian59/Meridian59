from fastapi import APIRouter
import api_dispatch  # Import the C extension functions

router = APIRouter()

@router.get("/players/online")
def get_online_players():
    """ Calls the C function to get online players as JSON """
    players = api_dispatch.get_online_players()  # Use the wrapper function
    return {"online_players": players}

@router.get("/server/status")
def get_server_status():
    """ Calls the C function to get server status as JSON """
    status = api_dispatch.get_server_status()  # Use the wrapper function
    return {"server_status": status}

@router.get("/signal/console")
def signal_console():
    """ Calls the C function to signal the console """
    result = api_dispatch.signal_console()  # Use the wrapper function
    return {"result": result}