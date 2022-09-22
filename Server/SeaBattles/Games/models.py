from django.db import models

# Create your models here.
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


class ShipPart(models.Model):
    x_pos = models.IntegerField()
    y_pos = models.IntegerField()
    is_damaged = models.BooleanField(default=False)

    ship = models.ForeignKey(Ship, on_delete=models.CASCADE)

    def __str__(self):
        return f"[{self.x_pos}, {self.y_pos}]"