from channels.generic.websocket import AsyncJsonWebsocketConsumer
from asgiref.sync import sync_to_async

from django.db.models import Q

import os
import django

from typing import Dict, Tuple, Union, Coroutine

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
django.setup()

from RestfulRequests.models import User, FriendRequest, Friends


NOT_ENOUGH_ARGUMENTS_JSON = {
    "error": "Недостаточно аргументов"
}
INVALID_ARGUMENTS_TYPE_JSON = {
    "error": "Неверный тип данных"
}


async def getUserIdByUsername(username: str) -> int:
    try:
        return int((await sync_to_async(User.objects.get)(user_name=username)).user_id)
    except User.DoesNotExist:
        return 0


class FriendRequestHandler():
    @staticmethod
    async def acceptFriendRequest(first_friend_id: int, second_friend_id: int) -> Tuple[bool, str]:
        friends = FriendRequest.objects.filter((Q(first_friend__user_id=first_friend_id) &
            Q(second_friend__user_name=second_friend_id.user_name)) | 
            (Q(first_friend__user_name=second_friend_id.user_name) &
            Q(second_friend__user_name=first_friend_id.user_name)))

    @staticmethod
    def declineFriendRequest(user_id: int, incoming_friend_name: str) -> Tuple[bool, str]:
        """
            Отменяет запрос в друзья

            Аргументы:
                user_id - Идентификатор пользователя
                incoming_friend_name - Имя друга, отправившего запрос в друзья
            
            Возвращает:
                Результат запроса и текст ошибки

        """
        ...

    @staticmethod
    async def sendFriendRequest(sender_id: int, receiver_id: int) -> Tuple[bool, str]:
        """
            Отправляет запрос в друзья пользователю

            Аргументы:
                sender_id - Идентификатор пользователя который отправляет запрос
                receiver_id - Идентификатор пользователя которому отправляется запрос
            
            Возвращает:
                Результат запроса и текст ошибки
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
            await sync_to_async(FriendRequest.objects.get)(from_user=from_user, to_user=to_user)
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
                Результат запроса и текст ошибки
            
            @TODO Удалять найденную связь друзей
        """       
        try:
            username: str = (await sync_to_async(User.objects.get)(user_id=user_id)).user_name
            await (sync_to_async(Friends.objects.filter)((Q(first_friend__user_name=friend_username) &
                                   Q(second_friend__user_name=username)) | 
                                   (Q(first_friend__user_name=username) &
                                   Q(second_friend__user_name=friend_username))))
        except User.DoesNotExist:
            return False, "Данного пользователя не существует"
        
        return True, ""


class FriendsUpdateConsumer(AsyncJsonWebsocketConsumer):
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    async def connect(self):
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        await self.accept()

    async def receive(self, text_data: str) -> Coroutine:
        """
            Получает информацию от сокета

            text_data может содержать следующие параметры
            action_type - Тип действия
            user_id - Идентификатор пользователя
            sender_id - Идентификатор пользователя, отправившего заявку в друзья
            receiver_id - Идентификатор пользователя, получившего заявку в друзья
            friend_username - Никнейм друга
            
            Аргументы:
                text_data - Полученная информация

            Возвращает:
                Текст ошибки или результат об успешной обработке
        """
        json_event = await self.decode_json(text_data)
        try:
            action_type = json_event["action_type"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            
        if action_type == "subscribe":
            try:
                user_id = int(json_event["user_id"])
                self.listeners[user_id] = self
                print(self.listeners)
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

            await self.send_json({"action_type": "subscribed"})
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
            
            await self.listeners[receiver_id].send({
                "action_type": "new_friend_request"
            })

            return await self.send_json({
                "action_type": "friend_request_sent"
            })
        elif action_type == "delete_friend":
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

            friend_id = await getUserIdByUsername(friend_username)
            await self.listeners[friend_id].send_json({
                "action_type": "deleted_by_friend"
            })

            await self.send_json({
                "action_type": "friend_deleted"
            })
        

        print(self.listeners)

    async def disconnect(self, event):
        """
            Обрабатывает поведение при отключении сокета от сервера

            @TODO Сделать менее затратный цикл(возможно перейти на обратный dict по ключам
            в виде сокетов и значений в виде user_id)
        """
        for user_id in self.listeners.keys():
            if self.listeners[user_id] == self:
                self.listeners.pop(user_id)
                break

        print(self.listeners)


class FriendlyDuelConsumer(AsyncJsonWebsocketConsumer):
    listeners:Dict[str, AsyncJsonWebsocketConsumer] = dict()
    async def connect(self) -> Coroutine:
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        await self.accept()

    async def receive(self, text_data: str) -> Coroutine:
        """
            Получает информацию от сокета

            text_data может содержать следующие параметры
            action_type - Тип действия
            user_id - Идентификатор пользователя
            from_user_id - Идентификатор пользователя, отправившего вызов на дуэль
            to_user_id - Идентификатор пользователя, получившего вызов на дуэль
            game_id - Идентификатор игры для дуэли
            
            Аргументы:
                text_data - Полученная информация

            Возвращает:
                Текст ошибки или результат об успешной обработке
        """
        json_event = await self.decode_json(text_data)
        action_type = json_event["action_type"]
        if action_type == "subscribe":
            user_id = json_event["user_id"]
            self.listeners[user_id] = self

            try:
                user = await sync_to_async(User.objects.get)(user_id=user_id)
            except User.DoesNotExist:
                return await self.send(text_data=await self.encode_json({
                    "error": "Данного пользователя не существует"
                }))

            await self.send(text_data=await self.encode_json({
                "user_id": str(user.user_id)
            }))
        elif action_type == "send_invite":
            from_user_id: str = json_event["from_user_id"]
            to_user_id: str = json_event["to_user_id"]
            game_id: str = json_event["game_id"]

            await self.listeners[to_user_id].send(text_data=await self.encode_json({
                "type": "game_invite", 
                "game_id": str(game_id), 
                "from_user_id": str(from_user_id)
            }))
        elif action_type == "accept_invite":
            from_user_id: str = json_event["from_user_id"]
            to_user_id: str = json_event["to_user_id"]
            await self.listeners[to_user_id].send(text_data=await self.encode_json({
                "type": "game_accept",
                "from_user_id":str(from_user_id)
            }))

            await self.send(text_data=await self.encode_json({
                "status": "accepted"
            }))

    async def disconnect(self, event):
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        print("Disconnected from server")
