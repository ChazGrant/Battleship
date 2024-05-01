from asgiref.sync import sync_to_async
from typing import Union
from hashlib import md5
from random import randint

from os import environ
from django import setup

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import User


async def hashPassword(email: str, username: str, password: str) -> str:
    salt = ""

    for part in range(0, len(email), 2):
        salt += email[part]
    for part in range(0, len(username), 2):
        salt += username[part]
    
    return md5((password + salt).encode()).hexdigest()


class UserDatabaseAccessor:
    @staticmethod
    async def getUserIdByUsername(username: str) -> int:
        try:
            return int((await sync_to_async(User.objects.get)(user_name=username)).user_id)
        except User.DoesNotExist:
            return 0

    @staticmethod
    async def deleteTemporaryUser(user_id: int) -> None:
        try:
            await sync_to_async(
                (await sync_to_async(User.objects.get)(user_id=user_id, is_temporary=True))
            .delete)()
        except User.DoesNotExist:
            ...

    @staticmethod
    async def getLastUserId() -> int:
        try:
            return (await sync_to_async(User.objects.latest)("user_id")).user_id
        except User.DoesNotExist:
            return 0

    @staticmethod
    async def createTemporaryUser(user_name: str, 
                         user_id: int, 
                         clean_password: str, 
                         user_email: str) -> User:
        try:
            await sync_to_async((await sync_to_async(User.objects.get)(
                user_name=user_name,
                user_email=user_email,
                is_temporary=True)).delete)()
        except User.DoesNotExist:
            ...
        
        hashed_password = await hashPassword(user_email, user_name, clean_password)
        created_user = await sync_to_async(User)(user_name=user_name, 
                            user_password=clean_password, 
                            user_email=user_email,
                            user_id=user_id,
                            is_temporary=True)
        
        await sync_to_async(created_user.full_clean)()
        created_user.user_password = hashed_password
        await sync_to_async(created_user.save)()

        return created_user

    @staticmethod
    async def getUserById(user_id: int) -> Union[User, None]:
        try:
            return await sync_to_async(User.objects.get)(user_id=user_id)
        except User.DoesNotExist:
            return None

    @staticmethod
    async def awardWinner(user_id: int) -> None:
        user = await UserDatabaseAccessor.getUserById(user_id)
        if not (user == None):
            user.silver_coins += 50
            user.win_streak += 1
            user.cups += randint(5, 8)
            await sync_to_async(user.save)()

    @staticmethod
    async def resetWinStreak(user_id: int) -> None:
        user = await UserDatabaseAccessor.getUserById(user_id)
        if not(user == None):
            user.win_streak = 0
            user.cups -= randint(5, 8)
            if user.cups < 0:
                user.cups = 0
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
