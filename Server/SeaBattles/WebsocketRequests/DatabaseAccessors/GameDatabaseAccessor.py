from os import environ
from django import setup
from asgiref.sync import sync_to_async
from typing import Tuple, List

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Field, Game
from WebsocketRequests.Generator import Generator
from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor


class GameDatabaseAccessor:
    @staticmethod
    async def createGame(creator_id: int, another_user_id: int) -> Tuple[int, str, str]:
        try:
            await sync_to_async(Field.objects.get)(owner_id=creator_id)
            return 0, "", "Игрок уже находится в игре"
        except Field.DoesNotExist:
            ...

        created_game = await sync_to_async(Game.objects.create)(
            game_id=await Generator.generateGameId(),
            user_id_turn=creator_id,
            is_friendly=True,
            game_invite_id=await Generator.generateGameInviteId(creator_id, another_user_id)
        )

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
