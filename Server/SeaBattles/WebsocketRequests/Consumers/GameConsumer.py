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
        ...

    async def subscribe(self, json_object: dict) -> None:
        """
            Обрабатывает поведение при отключении сокета от сервера
        """
        ...

    async def receive(self, json_object: dict) -> None:
        """
            Получает информацию от сокета

            text_data содержит action_type, отвечающий за тип действия
            action_type - Тип действия
            
            Аргументы:
                text_data - Полученная информация

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
