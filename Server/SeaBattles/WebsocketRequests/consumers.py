from channels.generic.websocket import AsyncJsonWebsocketConsumer, json
from asgiref.sync import sync_to_async

from django.db.models import Q

import os
import django
from random import randint
from hashlib import md5

from typing import Dict, Tuple, Coroutine, Callable

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
django.setup()

from RestfulRequests.models import User, FriendRequest, Friends, Game, Field
from RestfulRequests.views import MAX_LIMIT


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
USER_IS_OFFLINE_JSON = {
    "error": "Данного пользователя нет в сети"
}


class Generator:
    @staticmethod
    async def generateGameId() -> str:
        """
            Создаёт идентификатор игры

            Возврашает:
                Строку, содержащую идентификатор для созданной игры
        """
        game_id = ""
        for _ in range(8):
            game_id += await sync_to_async(str)(await sync_to_async(randint)(0, MAX_LIMIT))
        return game_id

    @staticmethod
    async def generateGameInviteId(first_user_id: int, second_user_id: int) -> str:
        """
            Создаёт идентификатор приглашения в игру

            Аргументы:
                first_user_id - Идентификатор первого пользователя
                second_user_id - Идентификатор второго пользователя
            
            Возвращает:
                Строку, содержащую идентификатор приглашения
        """
        return md5(
            (str(first_user_id) + str(second_user_id)).encode())\
        .hexdigest()


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


class FieldDatabaseAccessor:
    @staticmethod
    async def getFieldOwnerId(game: Game):
        try:
            return (await sync_to_async(Field.objects.get)(game=game)).owner_id, ""
        except Field.DoesNotExist:
            return 0, "Данного поля не существует"


class GameDatabaseAccessor:
    @staticmethod
    async def createGame(creator_id: int, another_user_id: int) -> Tuple[int, str]:
        try:
            await sync_to_async(Field.objects.get)(owner_id=creator_id)
            return 0, "Игрок уже находится в игре"
        except Field.DoesNotExist:
            ...

        created_game = await sync_to_async(Game.objects.create)(
            game_id=await Generator.generateGameId(),
            user_id_turn=creator_id,
            is_friendly=True,
            game_invite_id=await Generator.generateGameInviteId(creator_id, another_user_id)
        )

        return created_game.game_id, created_game.game_invite_id, ""

    @staticmethod
    async def getGameCreatorId(game_id: int) -> int:
        try:
            game = await sync_to_async(Game.objects.get)(game_id=game_id)
            return await FieldDatabaseAccessor.getFieldOwnerId(game)
        except Game.DoesNotExist:
            return 0


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
                Результат запроса и ошибку
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
                result, error = await FriendRequestDatabaseAccessor.processFriendRequest(to_user, from_user, True)
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

    async def receive(self, text_data: str) -> None:
        """
            Получает информацию от сокета

            text_data содержит action_type, отвечающий за тип действия
            action_type - Тип действия
            
            Аргументы:
                text_data - Полученная информация

            Возвращает:
                Текст ошибки или результат об успешной обработке
        """
        json_object = await self.decode_json(text_data)
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


class GameConsumer(AsyncJsonWebsocketConsumer):
    """
        Обработчик websocket запросов обработки игры

        Обрабатывает запросы, посылаемые на адрес ws://адрес_сервера/game_invite/

        @author     ChazGrant
        @version    1.0
    """
    groups = []
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    reversed_listeners: Dict[AsyncJsonWebsocketConsumer, int] = dict()
    def __init__(self):
        ...

    async def subscribe(self, json_object: dict) -> None:
        ...

    async def connectToGame(self, json_object: dict) -> None:
        ...

    async def shoot(self, json_object: dict) -> None:
        ...

    async def disconnect(self, event) -> None:
        ...


class FriendlyDuelConsumer(AsyncJsonWebsocketConsumer):
    """
        Обработчик websocket запросов обработки игры

        Обрабатывает запросы, посылаемые на адрес ws://адрес_сервера/friendly_duel/

        @author     ChazGrant
        @version    1.0
    """
    groups = []
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    reversed_listeners: Dict[AsyncJsonWebsocketConsumer, int] = dict()
    def __init__(self) -> None:
        """
            Конструктор класса

            Создаём список доступных действий и присваиваем им соответствующие методы
        """
        self._available_actions: Dict[str, Callable] = {
            "subscribe": self.subscribe,
            "send_game_invite": self.sendGameInvite
        }

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

    async def sendGameInvite(self, json_object: dict) -> None:
        """
            Отправляет приглашение в игру пользователю

            Аргументы:
                json_object - Словарь, содержащий идентификатор пользователя, который отправил запрос
                и идентификатор пользователя которому был отправлен запрос

            Возвращает:
                Ответ сокету, содержащий текст ошибки или информацию, что запрос был отправлен
        """
        try:
            from_user_id: int = int(json_object["from_user_id"])
            to_user_name: str = json_object["to_user_name"]
        except KeyError:       
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

        to_user_id = await UserDatabaseAccessor.getUserIdByUsername(to_user_name)

        if not await UserDatabaseAccessor.userExists(from_user_id) or \
           not to_user_id:
            return await self.send_json(USER_DOES_NOT_EXIST_JSON)

        if not (to_user_id in self.listeners.keys()):
            return await self.send_json(USER_IS_OFFLINE_JSON)
        
        game_id, game_invite_id, error = await GameDatabaseAccessor.createGame(
            creator_id=from_user_id,
            another_user_id=to_user_id
        )
        if not game_id:
            return await self.send_json({
                "error": error
            })

        if not(to_user_id in self.listeners.keys()):
            return await self.send_json(USER_IS_OFFLINE_JSON)

        await self.listeners[to_user_id].send_json({
            "action_type": "incoming_game_invite",
            "game_invite_id": game_invite_id,
            "game_id": str(game_id)
        })

        return await self.send_json({
            "action_type": "game_invite_sent",
            "game_id": str(game_id),
            "game_invite_id": game_invite_id
        })

    async def connect(self) -> None:
        """
            Обрабатывает поведение при подключении сокета к серверу
        """
        await self.accept()

    async def receive(self, text_data: str) -> None:
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
            json_object = await self.decode_json(text_data)
        except json.JSONDecodeError:
            return await self.send_json(INVALID_JSON_FORMATS)
        
        try:
            action_type = json_object["action_type"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        
        if action_type in self._available_actions.keys():
            return await self._available_actions[action_type](json_object)

        return await self.send_json(INVALID_ACTION_TYPE_JSON)

    async def disconnect(self, event) -> None:
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        print("Disconnected from server")
