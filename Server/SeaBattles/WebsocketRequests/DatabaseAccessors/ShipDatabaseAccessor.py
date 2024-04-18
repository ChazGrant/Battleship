from os import environ
from django import setup
from asgiref.sync import sync_to_async
from django.db.models.manager import BaseManager

from typing import List, Tuple

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
        ship_length_str = SHIP_LENGTHS_NAMES[ship_length]
        
        field = await FieldDatabaseAccessor.getField(user_id)
        if field == None:
            return False, "Данный игрок не имеет поля"
        
        if not (await FieldDatabaseAccessor.areShipsLeft(field, ship_length_str)):
            return False, "Данный тип кораблей закончился"

        ships = await ShipDatabaseAccessor.getShips(field)
        if await ShipDatabaseAccessor.hasCollisions(ships, cells):
            return False, "Обнаружена коллизия с другими кораблями"

        try:
            ship = await sync_to_async(Ship.objects.create)(field=field, ship_length=ship_length)
        except Exception:
            return False, "Неизвестная ошибка при создании корабля"
        
        for cell in cells:
            x, y = cell
            try:
                await sync_to_async(ShipPart.objects.create)(ship=ship, x_pos=x, y_pos=y)
            except Exception:
                await sync_to_async((await sync_to_async(ShipPart.objects.filter)(ship=ship)).delete)()
                await sync_to_async(ship.delete)()
                return False, "Невозможно создать часть корабля"

        FieldDatabaseAccessor.decreaseShipsAmount(field, ship_length_str)

        return True, ""
