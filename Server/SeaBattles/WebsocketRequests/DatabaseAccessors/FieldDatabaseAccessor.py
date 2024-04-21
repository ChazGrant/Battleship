from os import environ
from json import loads
from asgiref.sync import sync_to_async
from typing import Tuple, Union, List

from django import setup
from django.core.exceptions import ValidationError
from django.db.models import Count

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Field, Game, User

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.WeaponTypeDatabaseAccessor import WeaponTypeDatabaseAccessor


class FieldDatabaseAccessor:
    @staticmethod
    async def getFieldOwnerId(game: Game) -> Tuple[int, str]:
        try:
            return (await sync_to_async(Field.objects.get)(game=game)).owner.user_id, ""
        except Field.DoesNotExist:
            return 0, "Данного поля не существует"
        
    @staticmethod
    async def getFieldsParents() -> List[Tuple[int]]:
        return await sync_to_async((await sync_to_async(Field.objects.annotate)(
            article_count=Count('game_id')
        )).values_list)("game_id")

    @staticmethod
    async def allShipsArePlaced(user_id: int) -> Tuple[bool, str]:
        (one_deck, two_deck, three_deck, four_deck), error = \
            await FieldDatabaseAccessor.getShipsLeft(user_id)
        if error:
            return False, error
        return (one_deck + two_deck + three_deck + four_deck) == 0, ""
        
    @staticmethod
    async def checkForShipPartHit(user_id: int, x_pos: int, y_pos: int, weapon_name: str) \
                                            -> Tuple[List[List[List[int]]]]:
        field = FieldDatabaseAccessor.getField(user_id)
        x_range, y_range = WeaponTypeDatabaseAccessor.getWeaponRange(weapon_name)
        

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
            return (field.one_deck, field.two_deck, field.three_deck, field.four_deck), ""
        except Field.DoesNotExist:
            return (None, None, None, None), "Данный пользователь не имеет поля"
        
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

            return None, error_json["title"][0]

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
