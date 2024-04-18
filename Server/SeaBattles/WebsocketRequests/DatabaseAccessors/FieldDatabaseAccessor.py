from os import environ
from django import setup
from django.db.models import F
from django.core.exceptions import ValidationError
from asgiref.sync import sync_to_async

from typing import Tuple, Union

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Field, Game

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor

class FieldDatabaseAccessor:
    @staticmethod
    async def getFieldOwnerId(game: Game) -> Tuple[int, str]:
        try:
            return (await sync_to_async(Field.objects.get)(game=game)).owner_id, ""
        except Field.DoesNotExist:
            return 0, "Данного поля не существует"
        
    @staticmethod
    async def areShipsLeft(field: Field, ship_length_str: str) -> Tuple[bool, str]:
        return field.__dict__[ship_length_str] > 0

    @staticmethod
    async def createField(user_id: int, game: Game) -> Tuple[bool, str]:
        try:
            user = await UserDatabaseAccessor.getUserById(user_id)
            await sync_to_async(Field.objects.create)(owner=user, game=game)
            print("Field created")
            return True, ""
        except Exception:
            raise
        except ValidationError:
            return False, "Вы уже находитесь в игре"

    @staticmethod
    async def decreaseShipsAmount(field: Field, ship_length_str: str):
        await sync_to_async((await sync_to_async(Field.objects.filter)(owner_id=field.owner_id)).update)({
            ship_length_str: F(ship_length_str) - 1
        })

    @staticmethod
    async def getField(user_id: int) -> Union[Field, None]:
        try:
            return (await sync_to_async(Field.objects.get)(owner_id=user_id))
        except Field.DoesNotExist:
            return None
