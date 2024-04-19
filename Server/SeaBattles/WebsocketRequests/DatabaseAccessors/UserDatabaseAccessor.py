from asgiref.sync import sync_to_async

from os import environ
from django import setup

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import User


class UserDatabaseAccessor:
    @staticmethod
    async def getUserIdByUsername(username: str) -> int:
        try:
            return int((await sync_to_async(User.objects.get)(user_name=username)).user_id)
        except User.DoesNotExist:
            return 0

    @staticmethod
    async def getUserById(user_id: int) -> User:
        try:
            return await sync_to_async(User.objects.get)(user_id=user_id)
        except User.DoesNotExist:
            return None

    @staticmethod
    async def getUserByUsername(user_name: str) -> User:
        try:
            return await sync_to_async(User.objects.get)(user_name=user_name)
        except User.DoesNotExist:
            return None

    @staticmethod
    async def userExists(user_id: int) -> bool:
        try:
            await sync_to_async(User.objects.get)(user_id=user_id)
            return True
        except User.DoesNotExist:
            return False


    @staticmethod
    async def userExists(user_id: int) -> bool:
        try:
            await sync_to_async(User.objects.get)(user_id=user_id)
            return True
        except User.DoesNotExist:
            return False
