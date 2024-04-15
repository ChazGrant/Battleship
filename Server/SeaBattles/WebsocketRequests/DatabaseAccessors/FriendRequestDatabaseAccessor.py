from os import environ
from asgiref.sync import sync_to_async
from typing import Tuple

from django import setup
from django.db.models import Q

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import FriendRequest
from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.FriendDatabaseAccessor import FriendDatabaseAccessor
from WebsocketRequests.JSON_RESPONSES import USER_DOES_NOT_EXIST_JSON


class FriendRequestDatabaseAccessor:
    @staticmethod
    async def processFriendRequest(first_friend_id: int, second_friend_username: str, accept: bool) -> Tuple[bool, str]:
        """
            Принимает или отменяет запрос в друзья

            Аргументы:
                first_friend_id - Идентификатор первого друга
                second_friend_username - Имя пользователя второго друга

            Возвращает:
                Кортеж, содержащий результат выполнения и текст ошибки
        """
        second_friend_id = await UserDatabaseAccessor.getUserIdByUsername(second_friend_username)
        if second_friend_id == 0 or not (await UserDatabaseAccessor.userExists(first_friend_id)):
            return False, USER_DOES_NOT_EXIST_JSON["error"]
        
        first_user = await UserDatabaseAccessor.getUserById(first_friend_id)
        second_user = await UserDatabaseAccessor.getUserById(second_friend_id)
        
        friend_request = FriendRequest.objects.filter(
            (Q(from_user=first_user) & Q(to_user=second_user)) | 
            (Q(from_user=second_user) & Q(to_user=first_user)))
        
        if await sync_to_async(len)(friend_request) == 0:
            return False, "Данной заявки в друзья не существует"
        
        await sync_to_async(friend_request.delete)()
        if accept:
            await FriendDatabaseAccessor.createFriends(first_user, second_user)

        return True, ""

    @staticmethod
    async def sendFriendRequest(sender_id: int, receiver_id: int) -> Tuple[bool, str]:
        """
            Отправляет запрос в друзья пользователю

            Аргументы:
                sender_id - Идентификатор пользователя который отправляет запрос
                receiver_id - Идентификатор пользователя которому отправляется запрос
            
            Возвращает:
                Результат запроса и ошибку
        """
        from_user, error = await UserDatabaseAccessor.getUserById(sender_id)
        to_user, error = await UserDatabaseAccessor.getUserById(receiver_id)

        if from_user == None or to_user == None:
            return False, error
        
        friends = FriendDatabaseAccessor.getFriends()
        if await sync_to_async(len)(friends):
            return False, "Вы уже друзья с данным пользователем"

        try:
            await sync_to_async(FriendRequest.objects.get)(from_user=from_user, to_user=to_user)
            return False, "Вы уже отправили заявку этому пользователю"
        except FriendRequest.DoesNotExist:
            try:
                await sync_to_async(FriendRequest.objects.get)(from_user=to_user, to_user=from_user)
                result, error = await FriendRequestDatabaseAccessor.processFriendRequest(to_user, from_user, True)
                if not result:
                    return False, error
                else:
                    return True, ""
            except FriendRequest.DoesNotExist:
                await sync_to_async(FriendRequest.objects.create)(from_user=from_user, to_user=to_user)
                return True, ""
