from asgiref.sync import sync_to_async
from typing import Union

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
    async def getLastUserId() -> int:
        try:
            return (await sync_to_async(User.objects.latest)("user_id")).user_id
        except User.DoesNotExist:
            return 0

    @staticmethod
    async def createUser(user_name: str, 
                         user_id: int, 
                         clean_password: str, 
                         user_email: str,
                         is_temporary:bool=False) -> User:
        await sync_to_async(User.objects.create)()            
        hashed_password = hashPassword(email, user_name, clean_password)
        created_user = User(user_name=user_name, 
                            user_password=password, 
                            user_email=email,
                            user_id=last_user_id + 1)
        created_user.full_clean()
        created_user.user_password = hashed_password
        created_user.save()

    @staticmethod
    async def getUserById(user_id: int) -> Union[User, None]:
        try:
            return await sync_to_async(User.objects.get)(user_id=user_id)
        except User.DoesNotExist:
            return None

    @staticmethod
    async def awardSilverCoins(user_id: int) -> None:
        user = await UserDatabaseAccessor.getUserById(user_id)
        if not (user == None):
            user.silver_coins += 50
            user.win_streak += 1
            await sync_to_async(user.save)()

    @staticmethod
    async def resetWinStreak(user_id: int) -> None:
        user = await UserDatabaseAccessor.getUserById(user_id)
        if not(user == None):
            user.win_streak = 0
            await sync_to_async(user.save)()

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
