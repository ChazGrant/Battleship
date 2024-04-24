from asgiref.sync import sync_to_async
from typing import Dict, Tuple

from os import environ
from django import setup

environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')
setup()

from RestfulRequests.models import Weapon, WeaponType

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor


class WeaponDatabaseAccessor:
    @staticmethod
    async def getAvailableWeapons(user_id: int) -> Dict[str, Dict[str, int]]:
        user = await UserDatabaseAccessor.getUserById(user_id)
        weapons = await sync_to_async(Weapon.objects.filter)(weapon_owner=user)
        available_weapons: Dict[str, int] = dict()
        async for weapon in weapons:
            weapon_type: WeaponType = await sync_to_async(getattr)(weapon, "weapon_type")
            available_weapons[weapon_type.weapon_type_name] = {
                "weapon_amount": weapon.weapon_amount,
                "weapon_x_range": weapon.weapon_type.weapon_x_range,
                "weapon_y_range": weapon.weapon_type.weapon_y_range
            }

        return available_weapons

    @staticmethod
    async def decreaseWeaponAmount(user_id: int, weapon_name: str) -> int:
        user = await UserDatabaseAccessor.getUserById(user_id)
        weapon = await sync_to_async(Weapon.objects.get)(
            weapon_owner=user, 
            weapon_type__weapon_type_name=weapon_name
        )
        weapon.weapon_amount -= 1
        await sync_to_async(weapon.full_clean)()
        await sync_to_async(weapon.save)()

        return weapon.weapon_amount
    
    @staticmethod
    async def hasMassiveDamageProperty(weapon_name: str) -> bool:
        try:
            weapon_type = await sync_to_async(WeaponType.objects.get)(weapon_type_name=weapon_name)
        except WeaponType.DoesNotExist:
            return False
        
        return weapon_type.massive_damage

    @staticmethod
    async def getWeaponAmountLeft(user_id: int, weapon_name: str) -> int:
        user = await UserDatabaseAccessor.getUserById(user_id)
        try:
            weapon: Weapon = await sync_to_async(Weapon.objects.get)(
                weapon_type__weapon_type_name=weapon_name,
                weapon_owner=user
            )
        except Weapon.DoesNotExist:
            return 0
        
        return weapon.weapon_amount
