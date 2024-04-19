from os import environ
from typing import Tuple, List
from asgiref.sync import sync_to_async
from json import loads

from django import setup
from django.core.exceptions import ValidationError

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Game
from WebsocketRequests.Generator import Generator
from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor


class GameDatabaseAccessor:
    # *DEBUG
    @staticmethod
    async def deleteGames():
       await sync_to_async((await sync_to_async(Game.objects.all)()).delete)()

    @staticmethod
    async def getGame(game_id: str, game_invite_id:str="") -> Game:
        try:
            return await sync_to_async(Game.objects.get)(
                game_id=game_id,
                game_invite_id=game_invite_id)
        except Game.DoesNotExist:
            return None
    
    @staticmethod
    async def createGame(creator_id: int, another_user_id:int=0) -> Tuple[str, str, str]:
        field = await FieldDatabaseAccessor.getField(creator_id)
        if field:
            return "", "", "Игрок уже находится в игре"

        is_friendly = False
        game_invite_id = ""
        if another_user_id:
            is_friendly = True
            game_invite_id = await Generator.generateGameInviteId(creator_id, another_user_id)

        try:
            created_game = await sync_to_async(Game.objects.create)(
                game_id=await Generator.generateGameId(),
                user_id_turn=creator_id,
                is_friendly=is_friendly,
                game_invite_id=game_invite_id
            )
        except ValidationError as val_error:
            error_message = str(val_error).replace("'", "\"")
            error_json = loads(error_message)
            
            return False, error_json["title"][0]

        return created_game.game_id, created_game.game_invite_id, ""

    @staticmethod
    async def getGameCreatorId(game_id: int) -> int:
        try:
            game = await sync_to_async(Game.objects.get)(game_id=game_id)
            return await FieldDatabaseAccessor.getFieldOwnerId(game)
        except Game.DoesNotExist:
            return 0

    @staticmethod
    async def switchCurrentTurn(game_id: int) -> int:
        ...

    @staticmethod
    async def makeTurn(game_id: int, user_id: int, weapon_type: str, fire_position: List[int]):
        ...
