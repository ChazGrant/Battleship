from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict, Coroutine, Callable

from WebsocketRequests.JSON_RESPONSES import (NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON,
                            INVALID_ACTION_TYPE_JSON, USER_DOES_NOT_EXIST_JSON)

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.FriendRequestDatabaseAccessor import FriendRequestDatabaseAccessor


class FriendsUpdateConsumer(AsyncJsonWebsocketConsumer):
    """
        Обработчик websocket запросов действий с друзьями

        Обрабатывает запросы, посылаемые на адрес ws://адрес_сервера/friends_update/

        @author     ChazGrant
        @version    1.0
    """
    groups = []
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    reversed_listeners: Dict[AsyncJsonWebsocketConsumer, int] = dict()
    def __init__(self, *args, **kwargs):
        """
            Конструктор класса

            Создаём список доступных действий и присваиваем им соответствующие методы
        """
        self._available_actions: Dict[str, Callable] = {
            "subscribe": self.subscribe,
            "process_friend_request": self.processFriendRequest,
            "send_friend_request": self.sendFriendRequest,
            "delete_friend": self.deleteFriend      
        }

    async def connect(self):
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        await self.accept()

    async def subscribe(self, json_object: dict) -> None:
        """
            Подписывает сокет на события

            Аргументы:
                json_object - Словарь, содержащий идентификатор пользователя, который хочет подписаться

            Возвращает:
                Ответ сокету, содержащий текст ошибки или информацию, что он подписался
        """
        try:
            user_id = int(json_object["user_id"])
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
        
        if not (await UserDatabaseAccessor.userExists(user_id)):
            return await self.send_json(USER_DOES_NOT_EXIST_JSON)

        self.listeners[user_id] = self
        self.reversed_listeners[self] = user_id

        await self.send_json({
            "action_type": "subscribed"
        })

    async def processFriendRequest(self, json_object: dict) -> None:
        """
            Обрабатывает запрос в друзья

            Аргументы:
                json_object - Словарь, содержащий 
                имя пользователя друга,
                идентификатор пользователя, которому поступил запрос в друзья,
                статус обработки

            Возвращает:
                Ответ сокету, содержащий текст ошибки или информацию, что запрос обработан
        """
        try:
            friend_username = json_object["friend_username"]
            user_id = int(json_object["user_id"])
            process_status = int(json_object["process_status"])
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
                    
        result, error = await FriendRequestDatabaseAccessor.processFriendRequest(user_id, friend_username, process_status)

        if not result:
            return await self.send_json({
                "error": error
            })

        friend_id = await UserDatabaseAccessor.getUserIdByUsername(friend_username)
        if friend_id in self.listeners.keys():
            await self.listeners[friend_id].send_json({
                "action_type": "friend_request_processed"
            })

        await self.send_json({
            "action_type": "friend_request_processed"
        })

    async def sendFriendRequest(self, json_object: dict) -> None:
        """
            Отправляет запрос в друзья

            Аргументы:
                json_object - Словарь, содержащий 
                идентификатор получателя запроса в друзья,
                идентификатор отправителя запроса в друзья

            Возвращает:
                Ответ сокету, содержащий текст ошибки или информацию, что запрос отправлен
        """
        try:
            sender_id = int(json_object["sender_id"])
            receiver_id = int(json_object["receiver_id"])
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
        
        status, error = await FriendRequestDatabaseAccessor.sendFriendRequest(sender_id, receiver_id)
        if not status:
            return await self.send_json({
                "error": error
            })
        
        if receiver_id in self.listeners.keys():
            await self.listeners[receiver_id].send_json({
                "action_type": "new_friend_request"
            })

        await self.send_json({
            "action_type": "friend_request_sent"
        })

    async def deleteFriend(self, json_object: dict) -> None:
        """
            Удаляет друга

            Аргументы:
                json_object - Словарь, содержащий 
                идентификатор пользователя, который удаляет друга,
                имя пользователя, котрого нужно удалить из друзей

            Возвращает:
                Ответ сокету, содержащий текст ошибки или информацию, что друг удалён
        """
        try:
            user_id = int(json_object["user_id"])
            friend_username = json_object["friend_username"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
        
        status, error = await FriendRequestDatabaseAccessor.deleteFriend(user_id, friend_username)
        if not status:
            return await self.send_json({
                "error": error
            })

        friend_id = await UserDatabaseAccessor.getUserIdByUsername(friend_username)
        if friend_id == 0:
            return await self.send_json(USER_DOES_NOT_EXIST_JSON)
        
        if friend_id in self.listeners.keys():
            await self.listeners[friend_id].send_json({
                "action_type": "deleted_by_friend"
            })

        await self.send_json({
            "action_type": "friend_deleted"
        })

    async def receive_json(self, json_object: dict) -> None:
        """
            Получает информацию от сокета

            json_object содержит
            action_type - Тип действия

            Возвращает:
                Текст ошибки или результат об успешной обработке
        """
        try:
            action_type = json_object["action_type"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            
        if action_type in self._available_actions.keys():
            return await self._available_actions[action_type](json_object)
        
        await self.send_json(INVALID_ACTION_TYPE_JSON)

    async def disconnect(self, event) -> Coroutine:
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        user_id = self.reversed_listeners[self]
        self.listeners.pop(user_id)
        self.reversed_listeners.pop(self)

        print("User({}) has been disconnected".format(user_id))
