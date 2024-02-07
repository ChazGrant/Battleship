from Games.models import User, UserWeapons, Weapon, WeaponType


users = User.objects.all()
for user in users:
    print(user.user_id)
