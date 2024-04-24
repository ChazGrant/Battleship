from os import environ
from asgiref.sync import sync_to_async
from typing import List, Tuple
from json import loads
from asyncio import sleep

import random

from django import setup
from django.db.models.manager import BaseManager
from django.core.exceptions import ValidationError

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Field, Ship, ShipPart

from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor


MAX_ROW_SHIP_PART_PLACEMENT = 9
MAX_COLUMN_SHIP_PART_PLACEMENT = 9

SHIP_LENGTHS_NAMES = {
    1: "one_deck",
    2: "two_deck",
    3: "three_deck",
    4: "four_deck"
}


def coordinatesInOnePlace(ship_parts_coordinates: List[List[int]]) -> Tuple[bool]:
    x_one_place = all(map(lambda x: x == ship_parts_coordinates[0][0], 
                    [x[0] for x in ship_parts_coordinates]))
    y_one_place = all(map(lambda y: y == ship_parts_coordinates[0][1], 
                    [y[1] for y in ship_parts_coordinates]))
    
    return x_one_place, y_one_place

def isOutOfField(cells: List[List[int]]) -> bool:
    for x, y in cells:
        if x > MAX_ROW_SHIP_PART_PLACEMENT or y > MAX_COLUMN_SHIP_PART_PLACEMENT or \
        x < 0 or y < 0:
            return True
        
    return False

def isSequence(x_equal, y_equal, ship_parts_coordinates):            
    start_num = ship_parts_coordinates[0][not y_equal]
    for idx, coords in enumerate(ship_parts_coordinates):
        if not (start_num + idx == coords[x_equal]) and \
            not (start_num - idx == coords[x_equal]):
            return False

    return True


class ShipDatabaseAccessor:
    @staticmethod
    async def hasCollisions(ships: BaseManager[Ship], cells: List[List[int]]) -> bool:
        """
            Проверяет наличие кораблей в области вокруг указанных клеток

            Аргументы:
                ships - Список кораблей на поле
                cells - Список координат клеток, где пользователь хочет разместить следующий корабль

            Возвращает:
                True, если в области вокруг клеток есть корабль, иначе False
            
            @TODO Оптимизировать цикл поиска коллизий
        """
        # Поиск клеток, заполненных кораблями
        async for ship in ships:
            ship_parts = await sync_to_async(ShipPart.objects.filter)(ship=ship)
            async for ship_part in ship_parts:
                for cell in cells:
                    cell_x, cell_y = cell

                    if ship_part.x_pos in [cell_x - 1, cell_x + 1, cell_x] \
                    and ship_part.y_pos in [cell_y - 1, cell_y + 1, cell_y]:
                        return True

        return False

    @staticmethod
    async def generateShip(user_id: int, ship_length: int) -> List[int]:      
        async def generateCoordinates(ship_length: int):
            cells = []
            orientation = random.choice(['horizontal', 'vertical'])

            if orientation == 'horizontal':
                x = random.randint(0, 9 - ship_length)
                y = random.randint(0, 9 - 1)
                for i in range(ship_length):
                    cells.append([x + i, y])
            else:
                x = random.randint(0, 9 - 1)
                y = random.randint(0, 9 - ship_length)
                for i in range(ship_length):
                    cells.append([x, y + i])
                
            return cells

        field = await FieldDatabaseAccessor.getField(user_id)
        ships = await sync_to_async(Ship.objects.filter)(field=field)

        MAX_ITERATIONS = 30
        i = 0
        while True:
            if i > MAX_ITERATIONS:
                return None
            i += 1
            cells = await generateCoordinates(ship_length)
            if not await ShipDatabaseAccessor.hasCollisions(ships, cells):
                await FieldDatabaseAccessor.decreaseShipsAmount(field, SHIP_LENGTHS_NAMES[ship_length])
                ship = await sync_to_async(Ship.objects.create)(
                    ship_length=ship_length,
                    field=field
                )
                for cell in cells:
                    await sync_to_async(ShipPart.objects.create)(
                        x_pos=cell[0],
                        y_pos=cell[1],
                        ship=ship
                    )
                return cells
                
    @staticmethod
    async def getShips(field: Field):
        return await sync_to_async(Ship.objects.filter)(field=field)

    @staticmethod
    async def createShip(cells: List[List[int]], user_id: str) -> Tuple[bool, str]:
        """
            Создаёт корабль

            Аргументы:
                cells - Список координат частей корабля
                user_id - Пользователь, который создаёт корабль
                game_id - Идентификатор игры, в которой создаётся корабль

            Возвращает:
                Результат создания и текст ошибки
        """
        ship_length = len(cells)

        try:
            ship_length_str = SHIP_LENGTHS_NAMES[ship_length]
        except KeyError:
            return False, "Данного корабля не существует"
        
        # Проверка на валидность размещения кораблей
        if not ship_length == 1:
            x_equal, y_equal = coordinatesInOnePlace(cells)
            if not (x_equal + y_equal == 1):
                return False, "NOT coordinatesInOnePlace"
            is_sequence = isSequence(x_equal, y_equal, cells)
            if not is_sequence:
                return False, "NOT is_sequence"

        if isOutOfField(cells):
            return False, "Невозможно разместить корабль за пределами поля"

        field = await FieldDatabaseAccessor.getField(user_id)
        if field == None:
            return False, "Данный игрок не имеет поля"
        
        if not (await FieldDatabaseAccessor.areShipsLeft(field, ship_length_str)):
            return False, "Данный тип кораблей закончился"

        ships = await ShipDatabaseAccessor.getShips(field)
        if await ShipDatabaseAccessor.hasCollisions(ships, cells):
            return False, "Обнаружена коллизия с другими кораблями"

        try:
            ship = await sync_to_async(Ship)(field=field, ship_length=ship_length)
            await sync_to_async(ship.full_clean)()
            await sync_to_async(ship.save)()
        except ValidationError as val_error:
            error_message = str(val_error).replace("'", "\"")
            error_json = loads(error_message)
            
            return False, error_json["title"][0]
        
        for cell in cells:
            x, y = cell
            try:
                ship_part = await sync_to_async(ShipPart)(ship=ship, x_pos=x, y_pos=y)
                await sync_to_async(ship_part.full_clean)()
                await sync_to_async(ship_part.save)()
            except ValidationError as val_error:
                error_message = str(val_error).replace("'", "\"")
                error_json = loads(error_message)

                await sync_to_async(ship.delete)()
                
                return False, "Невозможно создать часть корабля:\n" + error_json["title"][0]
            except Exception:
                await sync_to_async(ship.delete)()
                return False, "Невозможно создать часть корабля"

        await FieldDatabaseAccessor.decreaseShipsAmount(field, ship_length_str)

        return True, ""

    @staticmethod
    async def allShipsAreDead(field: Field) -> bool:
        alive_ships = await sync_to_async(Ship.objects.filter)(
            field=field,
            is_dead=False
        )
        return await sync_to_async(len)(alive_ships) == 0

    @staticmethod
    async def markShipsCells(field: Field, 
                             x_start: int, x_end: int, 
                             y_start: int, y_end: int,
                             massive_damage: bool) -> \
                                Tuple[List[int], List[int], List[int]]:
        missed_cells = []
        dead_cells = []
        damaged_cells = []
        for x in range(x_start, x_end):
            for y in range(y_start, y_end):
                try:
                    ship_part = await sync_to_async(ShipPart.objects.get)(
                        ship__field=field,
                        x_pos=x,
                        y_pos=y,
                        is_damaged=False
                    )
                    ship_part.is_damaged = True
                    await sync_to_async(ship_part.save)()

                    ship: Ship = await sync_to_async(getattr)(ship_part, "ship")
                    dead_ship_parts = await sync_to_async(ShipPart.objects.filter)(
                        ship=ship,
                        is_damaged=True
                    )

                    if (await sync_to_async(len)(dead_ship_parts)) == ship.ship_length:
                        ship.is_dead = True
                        await sync_to_async(ship.save)()

                        dead_ship_parts = await sync_to_async(ShipPart.objects.filter)(
                            ship=ship
                        )
                        async for dead_ship_part in dead_ship_parts:
                            dead_cells.append([dead_ship_part.x_pos, dead_ship_part.y_pos])
                    else:
                        damaged_cells.append([x, y])
                    if not massive_damage:
                        break
                except ShipPart.DoesNotExist:
                    missed_cells.append([x, y])               

        return missed_cells, damaged_cells, dead_cells
