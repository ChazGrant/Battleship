from asgiref.sync import sync_to_async

from os import environ
from django import setup

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from typing import Tuple

from RestfulRequests.models import User
from WebsocketRequests.JSON_RESPONSES import USER_DOES_NOT_EXIST_JSON


class UserDatabaseAccessor:
    @staticmethod
    async def getUserIdByUsername(username: str) -> int:
        try:
            return int((await sync_to_async(User.objects.get)(user_name=username)).user_id)
        except User.DoesNotExist:
            return 0

    @staticmethod
    async def userExists(user_id: int) -> bool:
        try:
            await sync_to_async(User.objects.get)(user_id=user_id)
            return True
        except User.DoesNotExist:
            return False

    @staticmethod
    async def getUserById(user_id: int) -> Tuple[User, str]:
        try:
            return await sync_to_async(User.objects.get)(user_id=user_id), ""
        except User.DoesNotExist:
            return None, USER_DOES_NOT_EXIST_JSON["error"]

    @staticmethod
    async def userExists(user_id: int) -> bool:
        try:
            await sync_to_async(User.objects.get)(user_id=user_id)
            return True
        except User.DoesNotExist:
            return False
