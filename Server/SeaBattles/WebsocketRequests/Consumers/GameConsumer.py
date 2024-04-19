from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict, List, Callable

from WebsocketRequests.JSON_RESPONSES import (NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON,
                            INVALID_ACTION_TYPE_JSON, USER_DOES_NOT_EXIST_JSON, USER_IS_ALREADY_IN_GAME,
                            GAME_DOES_NOT_EXIST)

from WebsocketRequests.DatabaseAccessors.ShipDatabaseAccessor import ShipDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.GameDatabaseAccessor import GameDatabaseAccessor


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
            "subscribe": self.subscribe,
            "make_turn": self.makeTurn,
            "disconnect_from_the_game": self.disconnectFromTheGame,
            "place_ship": self.placeShip,
            "connect_to_game": self.connectToGame
        }

    async def placeShip(self, json_object: dict) -> None:
        """
            Устанавливает корабль в заданных координатах

            json_object содержит
            user_id - Идентификатор игрока,
            game_id - Идентификатор игры,
            cells - Клетки, в которых нужно поместить корабль
        """
        try:
            user_id = json_object["user_id"]
            raw_cells = json_object["cells"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
        
        try:
            cells = [list(map(int, raw_cell)) for raw_cell in raw_cells]
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

        if not await UserDatabaseAccessor.userExists(user_id):
            return await self.send_json(USER_DOES_NOT_EXIST_JSON)

        result, error = await ShipDatabaseAccessor.createShip(cells, user_id)
        if not result:
            return await self.send_json({
                "error": error
            })

        (one_deck, two_deck, three_deck, four_deck), error = await FieldDatabaseAccessor.getShipsLeft(user_id)
        if error:
            return await self.send_json({
                "error": error
            })
        
        await self.send_json({
            "action_type": "ship_placed",
            "ship_parts_pos": cells,
            "one_deck_left": one_deck,
            "two_deck_left": two_deck,
            "three_deck_left": three_deck,
            "four_deck_left": four_deck
        })

        if (one_deck + two_deck + three_deck + four_deck) == 0:
            return await self.send_json({
                "action_type": "all_ships_are_placed"
            })

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
        
        self.listeners[user_id] = self
        self.reversed_listeners[self] = user_id

        return await self.send_json({
            "action_type": "subscribed"
        })

    async def receive_json(self, json_object: dict) -> None:
        """
            Получает информацию от сокета

            json_object содержит action_type, отвечающий за тип действия
            action_type - Тип действия
            
            Аргументы:
                json_object - Полученная информация

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

    async def connectToGame(self, json_object: dict) -> None:
        """
            Подключает к игре

            Аргументы:
                json_object - Словарь, содержащий
                идентификатор пользователя, который подключается,
                идентификатор игры, к которой подключаются,
                идентификатор приглашения в игру
        """
        try:
            user_id = int(json_object["user_id"])
            game_id = json_object["game_id"]
            game_invite_id = json_object["game_invite_id"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

        if not (await UserDatabaseAccessor.userExists(user_id)):
            return await self.send_json(USER_DOES_NOT_EXIST_JSON)

        if not (await FieldDatabaseAccessor.getField(user_id)) == None:
            return await self.send_json(USER_IS_ALREADY_IN_GAME)

        game = await GameDatabaseAccessor.getGame(game_id, game_invite_id)
        if not game:
            return await self.send_json(GAME_DOES_NOT_EXIST)

        field, error = await FieldDatabaseAccessor.createField(user_id, game)
        if not field:
            return await self.send_json({
                "error": error
            })
        
        return await self.send_json({
            "action_type": "connected_to_game",
            "one_deck_left": field.one_deck,
            "two_deck_left": field.two_deck,
            "three_deck_left": field.three_deck,
            "four_deck_left": field.four_deck
        })

    async def disconnect(self, event) -> None:
        """
            Отключает сокет от сервера
        """
        await GameDatabaseAccessor.deleteGames()
