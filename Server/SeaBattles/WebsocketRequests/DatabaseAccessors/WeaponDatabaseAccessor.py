from asgiref.sync import sync_to_async
from typing import Dict

from os import environ
from django import setup

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Weapon, WeaponType

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor


class WeaponDatabaseAccessor:
    @staticmethod
    async def getAvailableWeapons(user_id: int) -> Dict[str, int]:
        user = await UserDatabaseAccessor.getUserById(user_id)
        weapons = await sync_to_async(Weapon.objects.filter)(weapon_owner=user)
        available_weapons: Dict[str, int] = dict()
        async for weapon in weapons:
            weapon_type: WeaponType = await sync_to_async(getattr)(weapon, "weapon_type")
            available_weapons[weapon_type.weapon_type_name] = weapon.weapon_amount

        return available_weapons
