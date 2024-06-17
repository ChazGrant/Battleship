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
    """
        Хэширует пароль

        Аргументы:
            email - Электронная почта пользователя

            username - Имя пользователя

            password - Пароль пользователя

        Возвращает:
            Захэшированный пароль
    """
    salt = ""

    for part in range(0, len(email), 2):
        salt += email[part]
    for part in range(0, len(username), 2):
        salt += username[part]
    
    return md5((password + salt).encode()).hexdigest()


class UserDatabaseAccessor:
    @staticmethod
    async def getUserIdByUsername(username: str) -> int:
        """
            Получает идентификатор пользователя по его имени

            Аргументы:
                username - Имя пользователя

            Возвращает:
                Идентификатор пользователя
        """
        try:
            return int((await sync_to_async(User.objects.get)(user_name=username)).user_id)
        except User.DoesNotExist:
            return 0

    @staticmethod
    async def deleteTemporaryUser(user_id: int) -> None:
        """
            Удаляет временного пользователя

            Аргументы:
                user_id - Идентификатор пользователя, которого нужно удалить
        """
        try:
            await sync_to_async(
                (await sync_to_async(User.objects.get)(user_id=user_id, is_temporary=True))
            .delete)()
        except User.DoesNotExist:
            ...

    @staticmethod
    async def getLastUserId() -> int:
        """
            Получает последний идентификатор пользователя

            Возвращает:
                Идентификатор пользователя, который был создан последним
        """
        try:
            return (await sync_to_async(User.objects.latest)("user_id")).user_id
        except User.DoesNotExist:
            return 0

    @staticmethod
    async def createTemporaryUser(user_name: str, user_id: int, clean_password: str, user_email: str) -> User:
        """
            Создаёт временного пользователя

            Аргументы:
                user_name - Имя пользователя

                user_id - Идентификатор пользователя

                clean_password - Чистый незахэшированный пароль

                user_email - Электронная почта пользователя
            Возвращает:
                Объект созданного пользователя
        """
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
    async def getUserById(user_id: int) -> User | None:
        """
            Получает пользователя по его идентификатору

            Аргументы:
                user_id - Идентификатор пользователя, которого нужно найти

            Возвращает:
                Объект пользователя, если такой существует или None
        """
        try:
            return await sync_to_async(User.objects.get)(user_id=user_id)
        except User.DoesNotExist:
            return None

    @staticmethod
    async def awardWinner(user_id: int) -> None:
        """
            Награждает пользователя, который победил в игре

            Аргументы:
                user_id - Идентификатор пользователя
        """
        user = await UserDatabaseAccessor.getUserById(user_id)
        if not (user == None):
            user.silver_coins += 50
            user.win_streak += 1
            user.cups += randint(5, 8)
            await sync_to_async(user.save)()

    @staticmethod
    async def resetWinStreak(user_id: int) -> None:
        """
            Обнуляет кол-во побед подряд у заданного пользователя

            Аргументы:
                user_id - Идентификатор пользователя
        """
        user = await UserDatabaseAccessor.getUserById(user_id)
        if not(user == None):
            user.win_streak = 0
            user.cups -= randint(5, 8)
            if user.cups < 0:
                user.cups = 0
            await sync_to_async(user.save)()

    @staticmethod
    async def getUserByUsername(user_name: str) -> User | None:
        """
            Получает пользователя по его имени

            Аргументы:
                user_name - Имя пользователя, которого нужно найти

            Возвращает:
                Объект пользователя, если он существует или None
        """
        try:
            return await sync_to_async(User.objects.get)(user_name=user_name)
        except User.DoesNotExist:
            return None

    @staticmethod
    async def userExists(user_id: int) -> bool:
        """
            Возвращает информацию о том существует ли заданный пользователь

            Аргументы:
                user_id - Идентификатор пользователя

            Возвращает:
                Информацию о том существует заданный пользователь или нет
        """
        try:
            await sync_to_async(User.objects.get)(user_id=user_id)
            return True
        except User.DoesNotExist:
            return False
