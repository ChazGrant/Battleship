from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict, List, Callable
from asyncio import sleep
import copy

from WebsocketRequests.JSON_RESPONSES import (NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON,
                            INVALID_ACTION_TYPE_JSON, USER_DOES_NOT_EXIST_JSON, USER_IS_ALREADY_IN_GAME,
                            GAME_DOES_NOT_EXIST, NOT_YOUR_TURN)

from WebsocketRequests.DatabaseAccessors.ShipDatabaseAccessor import ShipDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.GameDatabaseAccessor import GameDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.WeaponDatabaseAccessor import WeaponDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.WeaponTypeDatabaseAccessor import WeaponTypeDatabaseAccessor


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
            "connect_to_game": self.connectToGame,
            "generate_field": self.generateField,
            "get_weapons": self.getWeapons
        }

    async def getWeapons(self, json_object: dict) -> None:
        try:
            user_id = int(json_object["user_id"])
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)
        
        available_weapons = await WeaponDatabaseAccessor.getAvailableWeapons(user_id)

        return await self.send_json({
            "action_type": "available_weapons",
            "available_weapons": available_weapons
        })

    async def generateField(self, json_object: dict):
        print("generateField")
        user_id = int(json_object["user_id"])
        game_id = json_object["game_id"]

        ships_amounts = {
            1: 4,
            2: 3,
            3: 2,
            4: 1
        }

        copy_ships_amount = copy.deepcopy(ships_amounts)

        for ship_length, ship_amount in ships_amounts.items():
            for _ in range(ship_amount):
                cells = await ShipDatabaseAccessor.generateShip(user_id, ship_length)
                if cells == None:
                    return await self.send_json({
                        "action_type": "*DEBUG"
                    })
                copy_ships_amount[ship_length] = copy_ships_amount[ship_length] - 1
                await self.send_json({
                    "action_type": "ship_placed",
                    "ship_parts_pos": cells,
                    "one_deck_left": copy_ships_amount[1],
                    "two_deck_left": copy_ships_amount[2],
                    "three_deck_left": copy_ships_amount[3],
                    "four_deck_left": copy_ships_amount[4]
                })
                # await sleep(1)

        await self.send_json({
            "action_type": "all_ships_are_placed"
        })
        
        if await GameDatabaseAccessor.opponentPlacedAllShips(game_id, user_id):
            game = await GameDatabaseAccessor.getGameByPlayerId(user_id)
            opponent_id = await FieldDatabaseAccessor.getOpponentId(game, user_id)
            await self.send_json({
                "action_type": "game_started",
                "user_id_turn": game.user_id_turn
            })

            if opponent_id in self.listeners:
                await self.listeners[opponent_id].send_json({
                "action_type": "game_started",
                "user_id_turn": game.user_id_turn
                })


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
            game_id = json_object["game_id"]
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
            await self.send_json({
                "action_type": "all_ships_are_placed"
            })

            opponent_placed_all_ships, error = \
                    await GameDatabaseAccessor.opponentPlacedAllShips(game_id, user_id)
            if error:
                return await self.send_json({
                    "error": error
                })
            if opponent_placed_all_ships:
                game = await GameDatabaseAccessor.getGameByPlayerId(user_id)
                opponent_id = await FieldDatabaseAccessor.getOpponentId(game, user_id)
                await self.send_json({
                    "action_type": "game_started",
                    "user_id_turn": game.user_id_turn
                })

                if opponent_id in self.listeners.keys():
                    await self.listeners[opponent_id].send_json({
                    "action_type": "game_started",
                    "user_id_turn": game.user_id_turn
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
            x_pos, y_pos = map(int, json_object["shoot_position"])
            if "weapon_name" not in json_object.keys():
                weapon_name = ""
            else:
                weapon_name = json_object["weapon_name"]
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return await self.send_json(INVALID_ARGUMENTS_TYPE_JSON)

        # Проверяем осталось ли у пользователя это оружие
        if not weapon_name == "single_shoot":
            ...

        # Проверяем существует ли такая игра и такой пользователь
        user = await UserDatabaseAccessor.getUserById(user_id)
        if user == None:
            return await self.send_json(USER_DOES_NOT_EXIST_JSON)
        
        game = await GameDatabaseAccessor.getGame(game_id)
        if game == None:
            return await self.send_json(GAME_DOES_NOT_EXIST)

        # Проверяем чей сейчас ход
        if not (game.user_id_turn == user_id):
            return await self.send_json(NOT_YOUR_TURN)
        
        # Бомбим поле
        opponent_id = await FieldDatabaseAccessor.getOpponentId(game, user_id)
        # *DEBUG
        opponent_field = await FieldDatabaseAccessor.getField(user_id)
        x_range, y_range = await WeaponTypeDatabaseAccessor.getWeaponRange(weapon_name)

        missed_cells, damaged_cells, dead_cells = await ShipDatabaseAccessor.\
                markShipsCells(opponent_field, x_pos, x_pos+x_range, y_pos, y_pos+y_range)
        
        await FieldDatabaseAccessor.createMissedCells(opponent_field, missed_cells)
        if dead_cells:
            for x, y in await FieldDatabaseAccessor.createMissedCellsAroundDeadCells(
                opponent_field, dead_cells):
                missed_cells.append([x, y])

        # Если есть мёртвые корабли и нет повреждённых кораблей
        if dead_cells and not damaged_cells:
            all_ships_are_dead = await ShipDatabaseAccessor.allShipsAreDead(opponent_field)
            if all_ships_are_dead:
                await GameDatabaseAccessor.setWinner(user_id)

                if opponent_id in self.listeners.keys():
                    await self.listeners[opponent_id].send_json({
                        "action_type": "game_over",
                        "game_over_cause": "all_ships_are_dead",
                        "winner_id": user_id
                    })
                
                return await self.send_json({
                    "action_type": "game_over",
                    "game_over_cause": "all_ships_are_dead",
                    "winner_id": user_id
                })

        return await self.send_json({
            "action_type": "turn_made",
            "damaged_cells": damaged_cells,
            "dead_cells": dead_cells,
            "missed_cells": missed_cells
        })

    async def disconnectFromTheGame(self, json_object: dict) -> None:
        """
            Отключает пользователя от игры

            json_object содержит
            user_id - Идентификатор пользователя, который хочет отключиться
            game_id - Идентификатор игры
        """
        try:
            user_id = int(json_object["user_id"])
        except KeyError:
            return await self.send_json(NOT_ENOUGH_ARGUMENTS_JSON)
        
        game = await GameDatabaseAccessor.getGameByPlayerId(user_id)
        opponent_id = await FieldDatabaseAccessor.getOpponentId(game, user_id)
        GameDatabaseAccessor.setWinner(opponent_id)

        await self.listeners[opponent_id].send_json({
            "action_type": "game_over",
            "game_over_cause": "opponent_disconnected",
            "winner_id": opponent_id
        })
        
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
            # *DEBUG
            await GameDatabaseAccessor.deleteGames()
            # return await self.send_json(USER_IS_ALREADY_IN_GAME)

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
        user_id = self.reversed_listeners[self]
        await self.disconnectFromTheGame({"user_id": user_id})

        # self.listeners.pop(user_id)
        # self.reversed_listeners.pop(self)

        await GameDatabaseAccessor.deleteGames()
