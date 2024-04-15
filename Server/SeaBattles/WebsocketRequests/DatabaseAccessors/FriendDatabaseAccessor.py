from os import environ
from asgiref.sync import sync_to_async
from typing import Tuple

from django import setup
from django.db.models.manager import BaseManager
from django.db.models import Q

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Friends, User
from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor


class FriendDatabaseAccessor:
    @staticmethod
    async def createFriends(first_user: User, second_user: User) -> Tuple[bool, str]:
        """
            Создаёт друзей

            Аргументы:
                first_user - Первый друг
                second_user - Второй друг

            Возвращает:
                Результат, что друзья были созданы и текст ошибки
        """
        try:
            await sync_to_async(Friends.objects.create)(first_friend=first_user, second_friend=second_user)
            return True, ""
        except Exception as e:
            return False, str(e)

    @staticmethod
    async def getFriends(first_user: User, second_user: User) -> BaseManager[Friends]:
        """
            Получает друзей

            Аргументы:
                first_user - Первый друг
                second_user - Второй друг

            Возвращает:
                BaseManager, содержащий в себе объекты друзей
        """
        return await sync_to_async(Friends.objects.filter)((Q(first_friend__user_name=first_user.user_name) &
                    Q(second_friend__user_name=second_user.user_name)) | 
                    (Q(first_friend__user_name=second_user.user_name) &
                    Q(second_friend__user_name=first_user.user_name)))
    
    @staticmethod
    async def deleteFriend(user_id: int, friend_username: str) -> Tuple[bool, str]:
        """
            Удаляет друга из списка друзей

            Аргументы:
                user_id - Идентификатор пользователя, который хочет удалить друга
                friend_username - Имя пользователя, которого нужно удалить из списка друзей

            Возвращает:
                Результат запроса и текст ошибки
        """
        user, error = await UserDatabaseAccessor.getUserById(user_id)
        if user == None:
            return False, error
        
        await sync_to_async((await sync_to_async(Friends.objects.filter)(
                (Q(first_friend__user_name=friend_username) & Q(second_friend__user_name=user.user_name)) | 
                (Q(first_friend__user_name=user.user_name) & Q(second_friend__user_name=friend_username)))
            ).delete)()
        await FriendDatabaseAccessor.deleteFriend(user.user_name, friend_username)
        
        return True, ""        
