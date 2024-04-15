from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict, Coroutine, Callable

from WebsocketRequests.JSON_RESPONSES import (NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON,
                            INVALID_ACTION_TYPE_JSON, USER_DOES_NOT_EXIST_JSON)

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.FriendRequestDatabaseAccessor import FriendRequestDatabaseAccessor


class GameConsumer(AsyncJsonWebsocketConsumer):
    """
        Обработчик websocket запросов обработки игры

        Обрабатывает запросы, посылаемые на адрес ws://адрес_сервера/game/

        @author     ChazGrant
        @version    1.0
    """
    groups = []
    listeners: Dict[int, AsyncJsonWebsocketConsumer] = dict()
    reversed_listeners: Dict[AsyncJsonWebsocketConsumer, int] = dict()
    def __init__(self):
        """
            Конструктор класса

            Создаём список доступных действий и присваиваем им соответствующие методы
        """
        self._available_actions: Dict[str, Callable] = {
            "make_turn": self.makeTurn,
            "disconnect_from_the_game": self.disconnectFromTheGame
        }

    async def makeTurn(self, json_object: dict) -> None:
        """
            Делает ход игрока

            json_object содержит 
            shoot_position - Координаты, по которым стреляет пользователь
            weapon_type - Оружие, которое использует пользователь
            user_id - Идентификатор пользователя, который стреляет
            game_id - Идентификатор игры

            Возвращает:
                Словарь, содержащий ошибку, если ход невозможен, или клетки, которые помечены как промах, 
                попадание или уничтожение корабля
        """
        try:
            user_id = int(json_object["user_id"])
            game_id = str(int(json_object["game_id"]))
            x_pos, y_pos = map(int, str(json_object["shoot_position"]).split(" "))
            weapon_type = json_object["weapon_type"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
        
        # Проверяем осталось ли у пользователя это оружие

        # Проверяем существует ли такая игра и такой пользователь

        # Проверяем чей сейчас ход

        # Получаем поле противника

        # Бомбим поле

        # Проверяем на попадание

        # Если корабль уничтожен то проверяем сколько кораблей осталось

    async def disconnectFromTheGame(self, json_object: dict) -> None:
        """
            Отключает пользователя от игры

            json_object содержит
            user_id - Идентификатор пользователя, который хочет отключиться
            game_id - Идентификатор игры
        """
        try:
            user_id = int(json_object["user_id"])
            game_id = str(int(json_object["game_id"]))
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        
        # Устанавливаем победителем оппонента

        # Оппоненту отправляется сообщение, что игра закончена из-за выхода из игры

    async def subscribe(self, json_object: dict) -> None:
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        try:
            user_id = json_object["user_id"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)

    async def receive_json(self, json_object: dict) -> None:
        """
            Получает информацию от сокета

            text_data содержит action_type, отвечающий за тип действия
            action_type - Тип действия
            
            Аргументы:
                json_object - Полученная информация

            Возвращает:
                Текст ошибки или результат об успешной обработке
        """
        ...

    async def connectToGame(self, json_object: dict) -> None:
        """
            Подключает к игре

            Аргументы:
                json_object - Словарь, содержащий
                идентификатор пользователя, который подключается,
                идентификатор игры, к которой подключаются
        """
        ...

    async def shoot(self, json_object: dict) -> None:
        """
            Выстреливает по полю оппнента

            Аргументы:
                json_object - Словарь, содержащий
                идентификатор пользователя, который стреляет,
                идентификатор игры, в которой находится пользователь,
                координаты клеток, по которым стреляет пользователь,
                какое оружие используется
        """
        ...

    async def disconnect(self, event) -> None:
        """
            Отключает сокет от сервера
        """
        ...
