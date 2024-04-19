from os import environ
from asgiref.sync import sync_to_async
from typing import List, Tuple
from json import loads

from django import setup
from django.db.models.manager import BaseManager
from django.core.exceptions import ValidationError

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Field, Ship, ShipPart

from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor

SHIP_LENGTHS_NAMES = {
    1: "one_deck",
    2: "two_deck",
    3: "three_deck",
    4: "four_deck"
}


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
                raise
                return False, "Невозможно создать часть корабля"

        await FieldDatabaseAccessor.decreaseShipsAmount(field, ship_length_str)

        return True, ""
