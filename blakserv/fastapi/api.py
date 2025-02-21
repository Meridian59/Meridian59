from fastapi import APIRouter
import apifn

router = APIRouter()

@router.get("/players/online")
def get_online_players():
    """ Calls the C function to get online players as JSON """
    players = apifn.get_online_players()
    return {"online_players": players}