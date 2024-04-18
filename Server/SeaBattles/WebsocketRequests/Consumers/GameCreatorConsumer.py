from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict, Callable

from WebsocketRequests.JSON_RESPONSES import (NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON,
                            INVALID_ACTION_TYPE_JSON, USER_DOES_NOT_EXIST_JSON, USER_IS_OFFLINE_JSON,
                            USER_IS_ALREADY_IN_GAME)

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.GameDatabaseAccessor import GameDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor


class GameCreatorConsumer(AsyncJsonWebsocketConsumer):
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
            "create_game": self.createGame
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

    async def createGame(self, json_object: dict) -> None:
        """
            Создаёт игру

            Аргументы:
                json_object - Словарь, содержащий идентификатор пользователя, который создаёт игру

            Возвращает:
                Ответ сокету, в котором содержится идентификатор игры для подключения
        """
        try:
            user_id = int(json_object["user_id"])
            to_user_id = 0
        except KeyError:       
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
        
        user = await UserDatabaseAccessor.getUserById(user_id)
        if user == None:
            return await self.send_json(USER_DOES_NOT_EXIST_JSON)
        
        if "to_user_name" in json_object.keys():
            to_user_name = json_object["to_user_name"]
            to_user_id = await UserDatabaseAccessor.getUserIdByUsername(to_user_name)
            if to_user_id == 0:
                return await self.send_json(USER_DOES_NOT_EXIST_JSON)

        if to_user_id not in self.listeners.keys():
            return await self.send_json(USER_IS_OFFLINE_JSON)

        if await FieldDatabaseAccessor.getField(user_id) or \
            await FieldDatabaseAccessor.getField(to_user_id):
            return await self.send_json(USER_IS_ALREADY_IN_GAME)
        
        game_id, game_invite_id, error  = await GameDatabaseAccessor.createGame(user_id, to_user_id)
        if not (game_id):
            return await self.send_json({
                "error": error
            })
        
        if to_user_id:
            self.listeners[to_user_id].send_json({
                "action_type": "incoming_game_invite",
                "game_id": game_id,
                "game_invite_id": game_invite_id
            })

        return await self.send_json({
            "action_type": "game_created",
            "game_invite_id": game_invite_id,
            "game_id": game_id
        })

    async def connect(self) -> None:
        """
            Обрабатывает поведение при подключении сокета к серверу
        """
        await self.accept()

    async def receive_json(self, json_object: dict) -> None:
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
            action_type = json_object["action_type"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        
        if action_type in self._available_actions.keys():
            return await self._available_actions[action_type](json_object)

        await self.send_json(INVALID_ACTION_TYPE_JSON)

    async def disconnect(self, event) -> None:
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        print("Disconnected from server")
