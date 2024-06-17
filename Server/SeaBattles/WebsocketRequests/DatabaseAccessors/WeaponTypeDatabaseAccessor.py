from asgiref.sync import sync_to_async
from typing import Tuple

from os import environ
from django import setup

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import WeaponType


class WeaponTypeDatabaseAccessor:
    @staticmethod
    async def getWeaponRange(weapon_name: str) -> Tuple[int]:
        """
            Получает радиус поражения заданног орудия

            Аргументы:
                weapon_name - Наименование оружия
        
            Возвращает:
                Кортеж, содержащий дальность поражения орудия по x и y
        """
        if weapon_name == "":
            return 1, 1
        print(weapon_name)
        weapon_type = await sync_to_async(WeaponType.objects.get)(weapon_type_name=weapon_name)
        return weapon_type.weapon_x_range, weapon_type.weapon_y_range
