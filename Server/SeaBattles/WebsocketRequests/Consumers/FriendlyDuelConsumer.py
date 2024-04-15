from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict, Callable

from WebsocketRequests.JSON_RESPONSES import (NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON,
                            INVALID_ACTION_TYPE_JSON, USER_DOES_NOT_EXIST_JSON, USER_IS_OFFLINE_JSON)

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.GameDatabaseAccessor import GameDatabaseAccessor


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

    async def receive(self, json_object: dict) -> None:
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
