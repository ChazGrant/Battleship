from django.db import models
from django.contrib.postgres.fields import ArrayField
from django.core.validators import MinLengthValidator


"""
    ForeignKey = Many items to one item(many ship parts to one ship)
"""


class User(models.Model):
    user_name = models.CharField(max_length=20, validators=[MinLengthValidator(4)], unique=True)
    user_password = models.CharField(max_length=25, validators=[MinLengthValidator(8)])
    user_id = models.CharField(max_length=30, unique=True)
    user_email = models.CharField(max_length=40, validators=[MinLengthValidator(5)], unique=True)

    class Meta:
        verbose_name = "User"
        verbose_name_plural = "Users"
    
    def __str__(self):
        return f"{self.user_name} {self.user_id} {self.user_password} {self.user_email}"


class Game(models.Model):
    game_id = models.CharField(max_length=30)
    user_id_turn = models.CharField(max_length=30)
    game_is_over = models.BooleanField(default=False)
    has_winner = models.BooleanField(default=False)

    class Meta:
        verbose_name = "Game"
        verbose_name_plural = "Games"

    def __str__(self):
        pass
    

class Field(models.Model):
    owner_id = models.CharField(max_length=30)

    one_deck = models.IntegerField(default=4)
    two_deck = models.IntegerField(default=3)
    three_deck = models.IntegerField(default=2)
    four_deck = models.IntegerField(default=1)

    game = models.ForeignKey(Game, on_delete=models.CASCADE)


# Повреждённая клетка поля, которая не является часть корабля
class MarkedCell(models.Model):
    x_pos = models.IntegerField()
    y_pos = models.IntegerField()

    field = models.ForeignKey(Field, on_delete=models.CASCADE)


class Ship(models.Model):
    ship_length = models.IntegerField()
    is_dead = models.BooleanField(default=False)

    field = models.ForeignKey(Field, on_delete=models.CASCADE)

    class Meta:
        verbose_name = "Ship"
        verbose_name_plural = "Ships"


class ShipPart(models.Model):
    x_pos = models.IntegerField()
    y_pos = models.IntegerField()
    is_damaged = models.BooleanField(default=False)

    ship = models.ForeignKey(Ship, on_delete=models.CASCADE)

    def __str__(self):
        return f"[{self.x_pos}, {self.y_pos}]"


class UserWeapons(models.Model):
    user = models.ForeignKey(User, on_delete=models.CASCADE)


class WeaponType(models.Model):
    class Types(models.TextChoices):
        AIRPLANE = "Самолёт"
        NUKE_BOMB = "Ядерная бомба"

    weapon_type_name = models.CharField(max_length=50, choices=Types.choices)
    weapon_range = ArrayField(models.IntegerField(), blank=True)
    weapon_price = models.FloatField(default=0.0)


class Weapon(models.Model):
    weapon_amount = models.IntegerField(default=0)

    weapon_type = models.ForeignKey(WeaponType, on_delete=models.CASCADE)
    user_weapon = models.ForeignKey(UserWeapons, on_delete=models.CASCADE)


class Friends(models.Model):
    first_friend = models.ForeignKey(User, related_name='first_friend', on_delete=models.CASCADE)
    second_friend = models.ForeignKey(User, related_name='second_friend', on_delete=models.CASCADE)


class FriendRequest(models.Model):
    from_user = models.ForeignKey(User, related_name='from_user', on_delete=models.CASCADE)
    to_user = models.ForeignKey(User, related_name='to_user', on_delete=models.CASCADE)
