from Games.models import User, UserWeapons, Weapon, WeaponType

new_user = User.objects.create(user_name="admin", user_password="password", user_id=2, user_email="admin@admin.com")
new_user.save()

admin_weapons = UserWeapons.objects.create(user=new_user)

airplane_weapon = WeaponType.objects.create(weapon_type_name=WeaponType.Types.AIRPLANE, weapon_range=(9, 1), weapon_price=25.5)
airplane_weapon.save()

Weapon.objects.create(weapon_amount=3, weapon_type=airplane_weapon, user_weapon=admin_weapons).save()
