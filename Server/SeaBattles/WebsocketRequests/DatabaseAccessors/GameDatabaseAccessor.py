from os import environ
from typing import Tuple, List, Union
from asgiref.sync import sync_to_async
import asyncstdlib as a
from json import loads
from random import choice

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
    async def deleteGame(game: Game) -> None:
        print(game)
        await sync_to_async(game.delete())
        await sync_to_async(game.save)()

    @staticmethod
    async def getRandomWaitingGameId() -> Union[str, None]:
        game_ids = await FieldDatabaseAccessor.getFieldsParents()
        waiting_games_id = list()

        viewed_games = list()
        async for idx, unique_game_id in a.enumerate(game_ids):
            game_id = (await sync_to_async(Game.objects.get)(id=unique_game_id[0])).game_id
            if game_id not in game_ids[:idx] + game_ids[idx + 1:] \
                and game_id not in viewed_games:
                waiting_games_id.append(game_id)
            viewed_games.append(game_id)

        if len(waiting_games_id) == 0:
            return None

        return choice(waiting_games_id)

    @staticmethod
    async def setWinner(user_id: int) -> None:
        game = await GameDatabaseAccessor.getGameByPlayerId(user_id)
        game.game_is_over = True
        game.winner_id = user_id

        await sync_to_async(game.save)()

    @staticmethod
    async def getGame(game_id: str, game_invite_id:str="") -> Union[Game, None]:
        try:
            return await sync_to_async(Game.objects.get)(
                game_id=game_id,
                game_invite_id=game_invite_id)
        except Game.DoesNotExist:
            return None
    
    @staticmethod
    async def createGame(creator_id: int, opponent_is_ai: bool, another_user_id:int=0) -> Tuple[str, str, str]:
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
                game_invite_id=game_invite_id,
                opponent_is_ai=opponent_is_ai
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
    async def getGameByPlayerId(player_id: int) -> Union[Game, None]:
        field = await FieldDatabaseAccessor.getField(player_id)
        try:
            return await sync_to_async(getattr)(field, "game")
        except AttributeError:
            return None

    @staticmethod
    async def opponentPlacedAllShips(game_id: str, player_id: int) -> bool:
        game = await GameDatabaseAccessor.getGame(game_id)
        opponent_id = await FieldDatabaseAccessor.getOpponentId(game, player_id)

        if opponent_id == None:
            return False
        
        return await FieldDatabaseAccessor.allShipsArePlaced(opponent_id)

    @staticmethod
    async def switchCurrentTurn(game_id: int) -> None:
        game = await GameDatabaseAccessor.getGame(game_id)
        opponent_id = await FieldDatabaseAccessor.getOpponentId(game, game.user_id_turn)

        game.user_id_turn = opponent_id
        await sync_to_async(game.save)()

    @staticmethod
    async def getUserIdTurn(game_id: int) -> int:
        game = await GameDatabaseAccessor.getGame(game_id)
        return game.user_id_turn
