from os import environ
from json import loads
from asgiref.sync import sync_to_async
from typing import Tuple, Union, List

from django import setup
from django.core.exceptions import ValidationError
from django.db.models import Count

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Field, Game, User, MissedCell

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor


class FieldDatabaseAccessor:
    @staticmethod
    async def resetField(user_id: int) -> None:
        try:
            field = await FieldDatabaseAccessor.getField(user_id)

            field.one_deck = 4
            field.three_deck = 3
            field.two_deck = 2
            field.four_deck = 1
            await sync_to_async(field.save)()
        except Field.DoesNotExist:
            return

    @staticmethod
    async def getFieldsParents() -> List[Tuple[int]]:
        return await sync_to_async((await sync_to_async(Field.objects.annotate)(
            article_count=Count('game_id')
        )).values_list)("game_id")

    @staticmethod
    async def allShipsArePlaced(user_id: int) -> bool:
        (one_deck, two_deck, three_deck, four_deck) = \
            await FieldDatabaseAccessor.getShipsLeft(user_id)
        
        return (one_deck + two_deck + three_deck + four_deck) == 0
        
    @staticmethod
    async def createMissedCellsAroundDeadCells(field: Field, dead_cells: List[int]) -> List[int]:
        x_cells = [cell[0] for cell in dead_cells]
        y_cells = [cell[1] for cell in dead_cells]
        min_x = min(x_cells) - 1
        max_x = max(x_cells)
        min_y = min(y_cells) - 1
        max_y = max(y_cells)

        missed_cells:List[int] = list()
        for x in range(min_x, max_x + 2):
            for y in range(min_y, max_y + 2):
                if not(x < 0 or x > 9 or y < 0 or y > 9) and \
                    not(x in x_cells and y in y_cells):
                    try:
                        await sync_to_async((
                            await sync_to_async(MissedCell.objects.create)(
                                field=field,
                                x_pos=x,
                                y_pos=y)
                        ).full_clean)()
                        missed_cells.append([x, y])
                    except Exception:
                        raise

        return missed_cells

    @staticmethod
    async def createMissedCells(field: Field, missed_cells: List[int]) -> None:
        for x, y in missed_cells:
            try:
                await sync_to_async(MissedCell.objects.get)(field=field, x_pos=x, y_pos=y)
            except MissedCell.DoesNotExist:
                await sync_to_async(MissedCell.objects.create)(field=field, x_pos=x, y_pos=y)        

    @staticmethod
    async def getOpponentId(game: Game, player_id: int) -> Union[int, None]:
        player = await UserDatabaseAccessor.getUserById(player_id)
        found_fields = (await sync_to_async(
                        (await sync_to_async(Field.objects.filter)(game=game))
                                   .exclude)(owner=player))
        oponnent_owner_field = (await sync_to_async(found_fields.first)())
        try:
            opponent_owner: User = await sync_to_async(getattr)(oponnent_owner_field, "owner")
        except AttributeError:
            return None
        return opponent_owner.user_id

    @staticmethod
    async def getShipsLeft(user_id: int) -> Tuple[Tuple[int], str]:
        try:
            user = await UserDatabaseAccessor.getUserById(user_id)
            field = await sync_to_async(Field.objects.get)(owner=user)
            return (field.one_deck, field.two_deck, field.three_deck, field.four_deck)
        except Field.DoesNotExist:
            return (None, None, None, None)
        
    @staticmethod
    async def areShipsLeft(field: Field, ship_length_str: str) -> Tuple[bool, str]:
        return field.__dict__[ship_length_str] > 0

    @staticmethod
    async def createField(user_id: int, game: Game) -> Tuple[Field, str]:
        try:
            user = await UserDatabaseAccessor.getUserById(user_id)
            field = await sync_to_async(Field)(owner=user, game=game)
            await sync_to_async(field.full_clean)()
            await sync_to_async(field.save)()

            return field, ""
        except ValidationError as val_error:
            error_message = str(val_error).replace("'", "\"")
            error_json = loads(error_message)
            
            try:
                return None, error_json["title"][0]
            except KeyError:
                return None, error_json[list(error_json.keys())[0]][0]

    @staticmethod
    async def decreaseShipsAmount(field: Field, ship_length_str: str):
        current_value = getattr(field, ship_length_str)
        setattr(field, ship_length_str, current_value - 1)
        await sync_to_async(field.save)()

    @staticmethod
    async def getField(user_id: int) -> Union[Field, None]:
        try:
            user = await UserDatabaseAccessor.getUserById(user_id)
            return await sync_to_async(Field.objects.get)(owner=user)
        except Field.DoesNotExist:
            return None
