from channels.generic.websocket import AsyncJsonWebsocketConsumer, json
from asgiref.sync import sync_to_async

from django.db.models import Q

import os
import django
import random

from typing import Dict, Tuple, Coroutine

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
django.setup()

from RestfulRequests.models import User, FriendRequest, Friends, Game, Field
from RestfulRequests.views import MAX_LIMIT


async def generateGameId() -> str:
    """
        Создаёт идентификатор игры

        Возврашает:
            Строку, содержащую идентификатор для созданной игры
    """
    game_id = ""
    async for _ in range(8):
        game_id += await sync_to_async(str)(await sync_to_async(random.randint)(0, MAX_LIMIT))
    return game_id


NOT_ENOUGH_ARGUMENTS_JSON = {
    "error": "Недостаточно аргументов"
}
INVALID_ARGUMENTS_TYPE_JSON = {
    "error": "Неверный тип данных"
}
USER_DOES_NOT_EXIST_JSON = {
    "error": "Данного пользователя не существует"
}
INVALID_ACTION_TYPE_JSON = {
    "error": "Неизвестный тип действия"
}
INVALID_JSON_FORMATS = {
    "error": "Неверный формат json"
}


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
    async def getUserById(user_id: int) -> User:
        try:
            return await sync_to_async(User.objects.get)(user_id=user_id)
        except User.DoesNotExist:
            return USER_DOES_NOT_EXIST_JSON["error"]

    @staticmethod
    async def userExists(user_id: int) -> bool:
        try:
            await sync_to_async(User.objects.get)(user_id=user_id)
            return True
        except User.DoesNotExist:
            return False


class GameDatabaseAccessor:
    @staticmethod
    async def createGame(creator_id: int) -> Tuple[int, str]:
        try:
            await sync_to_async(Field.objects.get)(owner_id=creator_id)
            return 0, "Игрок уже находится в игре"
        except Field.DoesNotExist:
            ...

        created_game = await sync_to_async(Game.objects.create)(
            game_id=await generateGameId(),
            user_id_turn=creator_id,
            is_friendly=True
        )
        await sync_to_async(Field.objects.create)(owner_id=creator_id, game=created_game)

        return created_game.game_id, ""


class FriendRequestHandler():
    @staticmethod
    async def processFriendRequest(first_friend_id: int, second_friend_username: str, accept: bool) -> Tuple[bool, str]:
        """
            Принимает или отменяет запрос в друзья

            Аргументы:
                first_friend_id - Идентификатор первого друга
                second_friend_username - Имя пользователя второго друга

            Возвращает:
                Кортеж, содержащий результат выполнения и текс ошибки
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
            await sync_to_async(Friends.objects.create)(first_friend=first_user, second_friend=second_user)

        return True, ""

    @staticmethod
    async def sendFriendRequest(sender_id: int, receiver_id: int) -> Tuple[bool, str]:
        """
            Отправляет запрос в друзья пользователю

            Аргументы:
                sender_id - Идентификатор пользователя который отправляет запрос
                receiver_id - Идентификатор пользователя которому отправляется запрос
            
            Возвращает:
                Результат запроса и dict ошибки
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
                result, error = await FriendRequestHandler.processFriendRequest(to_user, from_user, True)
                if not result:
                    return False, error
                else:
                    return True, ""
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
            
            #TODO Удалять найденную связь друзей
        """
        try:
            username: str = (await sync_to_async(User.objects.get)(user_id=user_id)).user_name
            await sync_to_async((await sync_to_async(Friends.objects.filter)(
                (Q(first_friend__user_name=friend_username) & Q(second_friend__user_name=username)) | 
                (Q(first_friend__user_name=username) & Q(second_friend__user_name=friend_username)))
            ).delete)()
        except User.DoesNotExist:
            return False, "Данного пользователя не существует"
        
        return True, ""


class FriendsUpdateConsumer(AsyncJsonWebsocketConsumer):
    """
        Обработчик websocket запросов действий с друзьями

        Обрабатывает запросы, посылаемые на адрес ws://адрес_сервера/friends_update/

        @author     ChazGrant
        @version    1.0

        # TODO       Переделать if, else if... на словарь доступных команд и вызывать методы, привязанные
        к этим командам
    """
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    reversed_listeners: Dict[AsyncJsonWebsocketConsumer, int] = dict()
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
                if await UserDatabaseAccessor.userExists(user_id):
                    self.listeners[user_id] = self
                    self.reversed_listeners[self] = user_id
                else:
                    return await self.send_json(USER_DOES_NOT_EXIST_JSON)
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

            await self.send_json({"action_type": "subscribed"})
        elif action_type == "process_friend_request":
            try:
                friend_username = json_event["friend_username"]
                user_id = int(json_event["user_id"])
                process_status = int(json_event["process_status"])
            except KeyError:
                return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
                        
            result, error = await FriendRequestHandler.processFriendRequest(user_id, friend_username, process_status)

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
            
            if receiver_id in self.listeners.keys():
                await self.listeners[receiver_id].send_json({
                    "action_type": "new_friend_request"
                })

            await self.send_json({
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
        else:
            await self.send_json(INVALID_ACTION_TYPE_JSON)

        print(self.listeners)

    async def disconnect(self, event) -> Coroutine:
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        user_id = self.reversed_listeners[self]
        self.listeners.pop(user_id)
        self.reversed_listeners.pop(self)

        print("User({}) has been disconnected".format(user_id))


class GameConsumer(AsyncJsonWebsocketConsumer):
    """
        Обработчик websocket запросов действий с друзьями

        Обрабатывает запросы, посылаемые на адрес ws://адрес_сервера/game/

        @author     ChazGrant
        @version    1.0

        # TODO       Переделать if, else if... на словарь доступных команд и вызывать методы, привязанные
        к этим командам
    """
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    reversed_listeners: Dict[AsyncJsonWebsocketConsumer, int] = dict()
    async def connect(self) -> Coroutine:
        """
            Обрабатывает поведение при подключении сокета к серверу
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
        try:
            json_event = await self.decode_json(text_data)
        except json.JSONDecodeError:
            return await self.send_json(INVALID_JSON_FORMATS)
        
        try:
            action_type = json_event["action_type"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        
        if action_type == "subscribe":
            try:
                user_id = int(json_event["user_id"])
            except KeyError:
                return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
            
            if not (await UserDatabaseAccessor.userExists(user_id)):
                return await self.send_json(USER_DOES_NOT_EXIST_JSON)

            self.listeners[user_id] = self
            self.reversed_listeners[self] = user_id
        # TODO закончить логику
        elif action_type == "send_invite":
            try:
                from_user_id: int = int(json_event["from_user_id"])
                to_user_id: int = int(json_event["to_user_id"])
                game_id: int = int(json_event["game_id"])
            except KeyError:
                return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
            except ValueError:
                return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

            if not (to_user_id in self.listeners.keys()):
                return await self.send_json({
                    "error": "Данного пользователя нет в сети"
                })
            
            game_id, error = await GameDatabaseAccessor.createGame(creator_id=user_id)
            if game_id == 0:
                return await self.send_json({
                    "error": error
                })
            
            await self.listeners[to_user_id].send_json({
                "action_type": "game_invite", 
                "game_id": str(game_id)
            })

            return await self.send_json({
                "action_type": "game_invite_sent",
                "game_id": str(game_id)
            })
        elif action_type == "accept_invite":
            from_user_id: str = json_event["from_user_id"]
            to_user_id: str = json_event["to_user_id"]
            if to_user_id in self.listeners.keys():
                await self.listeners[to_user_id].send(text_data=await self.encode_json({
                    "type": "game_accept",
                    "from_user_id":str(from_user_id)
                }))

            await self.send(text_data=await self.encode_json({
                "status": "accepted"
            }))
        # Отправляет второму игроку что игра отменена
        # на клиенте закрывается окно с игрой и удаляется поле и игра
        elif action_type == "decline_invite":
            ...
        # Создаёт поле игры, отправляет второму игроку что игра начата
        # на клиенте запускает таймер ожидания хода
        elif action_type == "connect_to_game":
            ...
        else:
            return await self.send_json(INVALID_ACTION_TYPE_JSON)

    async def disconnect(self, event):
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        print("Disconnected from server")
