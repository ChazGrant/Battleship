from django.db import models
from django.core.validators import (MinLengthValidator, validate_email, MinValueValidator, 
                                    MaxValueValidator)
from django.core.exceptions import ValidationError


"""
    ForeignKey = Many items to one item(many ship parts to one ship)
"""


class User(models.Model):
    user_name = models.CharField(max_length=20, validators=[MinLengthValidator(4)], unique=True)
    user_password = models.CharField(max_length=25, validators=[MinLengthValidator(8)])
    user_id = models.IntegerField(unique=True)
    user_email = models.CharField(max_length=40, validators=[MinLengthValidator(5), validate_email], unique=True)

    silver_coins = models.FloatField(default=0, validators=[MinValueValidator(0)])
    win_streak = models.IntegerField(default=0, validators=[MinValueValidator(0)])

    is_temporary = models.BooleanField(default=False)

    cups = models.IntegerField(default=0, validators=[MinValueValidator(0)])

    class Meta:
        verbose_name = "User"
        verbose_name_plural = "Users"
    
    def __str__(self):
        return f"{self.user_name} {self.user_id} {self.user_password} {self.user_email}"


class PlayerLeague(models.Model):
    class LeagueCriteria:
        league_criteria = {
            "Бронзовая лига": [0, 50],
            "Серебряная лига": [50, 150],
            "Золотая лига": [150, 300],
            "Алмазная лига": [300, 9999],
        }
    league_name = models.CharField(max_length=15, validators=[MinLengthValidator(4)])

    min_cups_required = models.IntegerField(validators=[MinValueValidator(0)])
    max_cups_required = models.IntegerField(validators=[MinValueValidator(0)])

    def __str__(self):
        return f"Лига {self.league_name}: ({self.min_cups_required} - {self.max_cups_required})"
    

class Game(models.Model):
    game_id = models.CharField(max_length=30, unique=True)
    user_id_turn = models.IntegerField(unique=True)
    game_is_over = models.BooleanField(default=False)
    winner_id = models.IntegerField(null=True)

    is_friendly = models.BooleanField(default=False)
    game_invite_id = models.CharField(max_length=30, default="")

    opponent_is_ai = models.BooleanField(default=False)

    class Meta:
        verbose_name = "Game"
        verbose_name_plural = "Games"
    

class Field(models.Model):
    one_deck = models.IntegerField(default=4, validators=[MinValueValidator(0), MaxValueValidator(4)])
    two_deck = models.IntegerField(default=3, validators=[MinValueValidator(0), MaxValueValidator(3)])
    three_deck = models.IntegerField(default=2, validators=[MinValueValidator(0), MaxValueValidator(2)])
    four_deck = models.IntegerField(default=1, validators=[MinValueValidator(0), MaxValueValidator(1)])

    owner = models.OneToOneField(User, on_delete=models.CASCADE, unique=True)
    game = models.ForeignKey(Game, on_delete=models.CASCADE, related_name="fieldChild")


# Повреждённая клетка поля, которая не является часть корабля
class MissedCell(models.Model):
    x_pos = models.IntegerField(validators=[MinValueValidator(0), MaxValueValidator(9)])
    y_pos = models.IntegerField(validators=[MinValueValidator(0), MaxValueValidator(9)])

    field = models.ForeignKey(Field, on_delete=models.CASCADE)


class Ship(models.Model):
    ship_length = models.IntegerField()
    is_dead = models.BooleanField(default=False)

    field = models.ForeignKey(Field, on_delete=models.CASCADE)

    class Meta:
        verbose_name = "Ship"
        verbose_name_plural = "Ships"


class ShipPart(models.Model):
    x_pos = models.IntegerField(validators=[MinValueValidator(0), MaxValueValidator(9)])
    y_pos = models.IntegerField(validators=[MinValueValidator(0), MaxValueValidator(9)])
    is_damaged = models.BooleanField(default=False)

    ship = models.ForeignKey(Ship, on_delete=models.CASCADE)

    def __str__(self):
        return f"[{self.x_pos}, {self.y_pos}]"


class WeaponType(models.Model):
    class Types(models.TextChoices):
        AIRPLANE = "Самолёт"
        NUKE_BOMB = "Ядерная бомба"
        SMALL_BOMB = "Бомба"
        TIN_FISH = "Торпеда"

    weapon_type_name = models.CharField(max_length=30, choices=Types.choices, unique=True)
    weapon_x_range = models.IntegerField()
    weapon_y_range = models.IntegerField()
    weapon_price = models.FloatField(default=0.0)
    massive_damage = models.BooleanField(default=False)


class Weapon(models.Model):
    weapon_amount = models.IntegerField(default=0)

    weapon_type = models.ForeignKey(WeaponType, on_delete=models.CASCADE)
    weapon_owner = models.ForeignKey(User, on_delete=models.CASCADE)


class Friends(models.Model):
    first_friend = models.ForeignKey(User, related_name='first_friend', on_delete=models.CASCADE)
    second_friend = models.ForeignKey(User, related_name='second_friend', on_delete=models.CASCADE)

    def full_clean(self):
        if self.first_friend == self.second_friend:
            raise ValidationError({
                "title": "Вы не можете добавить самого себя в друзья"
            })

class FriendRequest(models.Model):
    from_user = models.ForeignKey(User, related_name='from_user', on_delete=models.CASCADE)
    to_user = models.ForeignKey(User, related_name='to_user', on_delete=models.CASCADE)

    def full_clean(self):
        if self.from_user == self.to_user:
            raise ValidationError({
                "title": "Вы не можете добавить самого себя в друзья"
            })
