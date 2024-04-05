from channels.generic.websocket import AsyncJsonWebsocketConsumer
from asgiref.sync import sync_to_async

from django.db.models import Q

import os
import django

from typing import Dict, Tuple, Union, Coroutine

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
django.setup()

from RestfulRequests.models import User, FriendRequest, Friends


listeners:Dict[str, AsyncJsonWebsocketConsumer] = dict()
NOT_ENOUGH_ARGUMENTS_JSON = {
    "error": "Недостаточно аргументов"
}
INVALID_ARGUMENTS_TYPE_JSON = {
    "error": "Неверный тип данных"
}

class FriendRequestHandler():
    # @staticmethod
    # async def acceptFriendRequest(first_friend_id: int, second_friend_id: int) -> Tuple[bool, str]:
    #     friends = FriendRequest.objects.filter((Q(first_friend__user_id=first_friend_id) &
    #         Q(second_friend__user_name=to_user.user_name)) | 
    #         (Q(first_friend__user_name=to_user.user_name) &
    #         Q(second_friend__user_name=from_user.user_name)))

    @staticmethod
    def declineFriendRequest():
        ...

    @staticmethod
    async def sendFriendRequest(sender_id: int, receiver_id: int) -> Tuple[bool, str]:
        """
            Отправляет запрос в друзья пользователю

            Аргументы:
                sender_id - Идентификатор пользователя который отправляет запрос
                receiver_id - Идентификатор пользователя которому отправляется запрос
            
            Возвращает:
                True если запрос отправлен, иначе False и текст ошибки
        """
        try:
            from_user = await sync_to_async(User.objects.get)(user_id=sender_id)
            to_user = await sync_to_async(User.objects.get)(user_id=receiver_id)
        except User.DoesNotExist:
            return False, "Одного из пользователей не существует"
        
        friends = await sync_to_async(Friends.objects.filter)((Q(first_friend__user_name=from_user.user_name) &
                    Q(second_friend__user_name=to_user.user_name)) | 
                    (Q(first_friend__user_name=to_user.user_name) &
                    Q(second_friend__user_name=from_user.user_name)))
        if await sync_to_async(len)(friends):
            return False, "Вы уже друзья с данным пользователем"

        try:
            sync_to_async(FriendRequest.objects.get)(from_user=from_user, to_user=to_user)
            return False, "Вы уже отправили заявку этому пользователю"
        except FriendRequest.DoesNotExist:
            try:
                await sync_to_async(FriendRequest.objects.get)(from_user=to_user, to_user=from_user)
                if await sync_to_async(FriendRequestHandler.acceptFriendRequest)(to_user, from_user):
                    return True, ""
                else:
                    return False, "Во время принятия запроса дружбы возникла ошибка"
            except FriendRequest.DoesNotExist:
                await sync_to_async(FriendRequest.objects.create)(from_user=from_user, to_user=to_user)
                return True, ""

    @staticmethod
    async def deleteFriend(user_id: int, friend_username: str) -> Tuple[bool, str]:
        """
            Удаляет друга из списка друзей

            Аргументы:
                user_id - Идентификатор пользователя, который хочет удалить друга
                friend_username - Имя пользователя, которого нужно удалить из списка друзей

            Возвращает:
                True если удаление успешно, иначе False и текст ошибки
        """       
        try:
            username: str = await sync_to_async(User.objects.get)(user_id=user_id).user_name
            Friends.objects.filter((Q(first_friend__user_name=friend_username) &
                                   Q(second_friend__user_name=username)) | 
                                   (Q(first_friend__user_name=username) &
                                   Q(second_friend__user_name=friend_username)))
        except User.DoesNotExist:
            return await False, "Данного пользователя не существует"
        
        return await True, ""


class FriendsUpdateConsumer(AsyncJsonWebsocketConsumer):
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    async def connect(self):
        await self.accept()

    async def receive(self, text_data: str) -> Coroutine:
        json_event = await self.decode_json(text_data)
        try:
            action_type = json_event["action_type"]
        except KeyError:
            self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            
        if action_type == "subscribe":
            try:
                user_id = int(json_event["user_id"])
                self.listeners[user_id] = self
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

            await self.send_json({"subscribed": True})
        elif action_type == "accept_friend_request":
            ...
        elif action_type == "decline_friend_request":
            ...
        elif action_type == "send_friend_request":
            try:
                sender_id = int(json_event["sender_id"])
                receiver_id = int(json_event["receiver_id"])
            except KeyError:
                return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
            
            status, error = await FriendRequestHandler.sendFriendRequest(sender_id, receiver_id)
            if not status:
                return await self.send_json({
                    "error": error
                })

            return await self.send_json({
                "friend_request_sent": True
            })
        elif action_type == "delete_friend":
            return
            try:
                user_id = int(json_event["user_id"])
                friend_username = json_event["friend_username"]
            except KeyError:
                return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
            
            status, error = await FriendRequestHandler.deleteFriend(user_id, friend_username)
            if not status:
                return await self.send_json({
                    "error": error
                })

            await self.send_json({
                "friend_deleted": True
            })
        

        print(self.listeners)

    async def disconnect(self, event):
        for user_id in self.listeners.keys():
            if self.listeners[user_id] == self:
                self.listeners.pop(user_id)
                break

        print(self.listeners)


class FriendlyDuelConsumer(AsyncJsonWebsocketConsumer):
    async def connect(self):
        await self.accept()

    async def receive(self, text_data):
        json_event = await self.decode_json(text_data)
        message_type = json_event["message_type"]
        if message_type == "subscribe":
            user_id = json_event["user_id"]
            listeners[user_id] = self

            try:
                user = await sync_to_async(User.objects.get)(user_id=user_id)
            except User.DoesNotExist:
                return await self.send(text_data=await self.encode_json({
                    "error": "Данного пользователя не существует"
                }))

            await self.send(text_data=await self.encode_json({
                "user_id": str(user.user_id)
            }))
        elif message_type == "send_invite":
            from_user_id: str = json_event["from_user_id"]
            to_user_id: str = json_event["to_user_id"]
            game_id: str = json_event["game_id"]

            await listeners[to_user_id].send(text_data=await self.encode_json({
                "type": "game_invite", 
                "game_id": str(game_id), 
                "from_user_id": str(from_user_id)
            }))
        elif message_type == "accept_invite":
            from_user_id: str = json_event["from_user_id"]
            to_user_id: str = json_event["to_user_id"]
            await listeners[to_user_id].send(text_data=await self.encode_json({
                "type": "game_accept",
                "from_user_id":str(from_user_id)
            }))

            await self.send(text_data=await self.encode_json(
            {
                "status": "accepted"
            }))

    async def disconnect(self, event):
        print("Disconnected from server")
