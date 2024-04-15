from os import environ
from django import setup
from asgiref.sync import sync_to_async

from typing import Tuple

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Field, Game


class FieldDatabaseAccessor:
    @staticmethod
    async def getFieldOwnerId(game: Game) -> Tuple[int, str]:
        try:
            return (await sync_to_async(Field.objects.get)(game=game)).owner_id, ""
        except Field.DoesNotExist:
            return 0, "Данного поля не существует"
