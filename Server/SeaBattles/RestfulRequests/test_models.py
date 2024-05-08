from django.test import TestCase
from django.core.exceptions import ValidationError
from django.db.utils import IntegrityError

from asgiref.sync import sync_to_async

from RestfulRequests.models import User, Field, Game, ShipPart, FriendRequest, WeaponType, Weapon, Ship

from typing import List
import random
import asyncstdlib as a

from WebsocketRequests.DatabaseAccessors.FieldDatabaseAccessor import FieldDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.WeaponDatabaseAccessor import WeaponDatabaseAccessor

from asgiref.sync import sync_to_async

# Create your tests here.
class UserModelTest(TestCase):
    def testShortUsername(self):
        user = User(
            user_name="a", 
            user_password="fasfasfjff", 
            user_id="2",
            user_email="bot@bot.bot"
        )

        self.assertRaises(ValidationError, user.full_clean)
        user.user_name = "username"
        self.assertEqual(user.full_clean(), None)

    def testShortPassword(self):
        user = User(
            user_name="bott", 
            user_password="a", 
            user_id="2",
            user_email="bot@bot.bot"
        )

        self.assertRaises(ValidationError, user.full_clean)
        user.user_password = "password"
        self.assertEqual(user.full_clean(), None)

    def testShortEmail(self):
        user = User(
            user_name="bott", 
            user_password="fasfasfjff", 
            user_id="2",
            user_email="a"
        )

        self.assertRaises(ValidationError, user.full_clean)
        user.user_email = "afskfnasiofn@mail.ru"
        self.assertEqual(user.full_clean(), None)
    
    def testLongUsername(self):
        user = User(
            user_name="".join(["a" for _ in range(25)]), 
            user_password="password", 
            user_id="2",
            user_email="bot@bot.bot"
        )

        self.assertRaises(ValidationError, user.full_clean)
        user.user_name = "user"
        self.assertEqual(user.full_clean(), None)

    def testLongPassword(self):
        user = User(
            user_name="username", 
            user_password="".join(["a" for _ in range(30)]), 
            user_id="2",
            user_email="bot@bot.bot"
        )

        self.assertRaises(ValidationError, user.full_clean)
        user.user_password = "password"
        self.assertEqual(user.full_clean(), None)
    
    def testLongEmail(self):
        user = User(
            user_name="username", 
            user_password="password", 
            user_id="2",
            user_email="".join(["a" for _ in range(45)])
        )

        self.assertRaises(ValidationError, user.full_clean)
        user.user_email = "email@email.email"
        self.assertEqual(user.full_clean(), None)

    def testOnUniqueUsername(self):
        User.objects.create(
            user_name="bott",
            user_password="password",
            user_email="bott@bot.bot",
            user_id=1
        )

        user = User(
            user_name="bott",
            user_password="password",
            user_email="bott@botff.bot",
            user_id=2
        )
        self.assertRaises(ValidationError, user.full_clean)
        user.user_name = "bottt"
        self.assertEqual(user.full_clean(), None)

    def testOnUniqueEmail(self):
        User.objects.create(
            user_name="bott",
            user_password="password",
            user_email="bott@bot.bot",
            user_id=1
        )

        user = User(
            user_name="bottf",
            user_password="password",
            user_email="bott@bot.bot",
            user_id=2
        )
        
        self.assertRaises(ValidationError, user.full_clean)
        user.user_email = "bot@bot.bot"
        self.assertEqual(user.full_clean(), None)

    def testCorrectEmail(self):
        user = User(
            user_name="username", 
            user_password="fasfasfjff", 
            user_id="2",
            user_email="bobot@bort"
        )

        self.assertRaises(ValidationError, user.full_clean)
        user.user_email = "bobotbprt.ru"
        self.assertRaises(ValidationError, user.full_clean)
        user.user_email = "bot@bot.bot"
        self.assertEqual(user.full_clean(), None)

    def testOnUniqueId(self):
        User.objects.create(
            user_name="bott",
            user_password="password",
            user_email="bott@bot.bot",
            user_id=1
        )

        user = User(
            user_name="bottff",
            user_password="password",
            user_email="bott@botff.bot",
            user_id=1
        )
        self.assertRaises(ValidationError, user.full_clean)
        user.user_id = 2
        self.assertEqual(user.full_clean(), None)


class FieldModelTest(TestCase):
    async def testDummy(self):
        return
        game = await sync_to_async(Game.objects.create)(
            game_id="12951925u9125asf",
            user_id_turn=1
        )

        user = await sync_to_async(User.objects.create)(
            user_id=1,
            user_name="username",
            user_password="password",
            user_email="bot@bot.ru"
        )

        user2 = await sync_to_async(User.objects.create)(
            user_id=2,
            user_name="username2",
            user_password="password2",
            user_email="bot2@bot.ru"
        )

        await sync_to_async(Field.objects.create)(
            owner=user,
            game=game
        )
        await sync_to_async(Field.objects.create)(
            owner=user2,
            game=game
        )

        player = await UserDatabaseAccessor.getUserById(user.user_id)
        found_fields = (await sync_to_async(
                        (await sync_to_async(Field.objects.filter)(game=game))
                                   .exclude)(owner=player))
        owner_field = (await sync_to_async(found_fields.first)())
        owner: User = await sync_to_async(getattr)(owner_field, "owner")
    
    def testOnUniqueOwner(self):
        game = Game.objects.create(
            game_id="12951925u9125asf",
            user_id_turn=1
        )

        user = User.objects.create(
            user_id=1,
            user_name="username",
            user_password="password",
            user_email="bot@bot.ru"
        )

        Field.objects.create(
            owner=user,
            game=game
        )

        new_field = Field(
            owner=user,
            game=game
        )

        self.assertRaises(ValidationError, new_field.full_clean)
        new_user = User.objects.create(
            user_id=2,
            user_name="username2",
            user_password="password2",
            user_email="bot2@bot.ru"
        )
        new_field.owner = new_user
        self.assertEqual(new_field.full_clean(), None)

    def testOnEmptyOwner(self):
        game = Game.objects.create(
            game_id="12951925u9125asf",
            user_id_turn=1
        )

        with self.assertRaises(IntegrityError):
            Field.objects.create(
                game=game
            )


class ShipModelTest(TestCase):
    def testCreation(self):
        with self.assertRaises(ValueError):
            ShipPart(ship=5, x_pos=2, y_pos=3)
        
        with self.assertRaises(ValueError):
            ShipPart(ship=5, x_pos=2, y_pos="f")

        with self.assertRaises(ValueError):
            ShipPart(ship=5, x_pos="f", y_pos=3)

    def coordinatesInOnePlace(self, ship_parts_coordinates):
        x_one_place = all(map(lambda x: x == ship_parts_coordinates[0][0], 
                        [x[0] for x in ship_parts_coordinates]))
        y_one_place = all(map(lambda y: y == ship_parts_coordinates[0][1], 
                        [y[1] for y in ship_parts_coordinates]))
        
        return x_one_place, y_one_place

    def testShipPartsSequence(self):
        def isSequence(x_equal, y_equal, ship_parts_coordinates):            
            start_num = ship_parts_coordinates[0][not y_equal]
            for idx, coords in enumerate(ship_parts_coordinates):
                if not (start_num + idx == coords[x_equal]) and \
                   not (start_num - idx == coords[x_equal]):
                    return False

            return True
        
        ship_parts_coordinates = [
            [1, 10],
            [1, 9],
            [1, 8]
        ]

        x_in_one_place, y_in_one_place = self.coordinatesInOnePlace(ship_parts_coordinates)
        is_sequence = isSequence(x_in_one_place, y_in_one_place, ship_parts_coordinates)
        self.assertEqual(is_sequence, True)

        ship_parts_coordinates = [[coords[1], coords[0]] for coords in ship_parts_coordinates]

        x_in_one_place, y_in_one_place = self.coordinatesInOnePlace(ship_parts_coordinates)
        is_sequence = isSequence(x_in_one_place, y_in_one_place, ship_parts_coordinates)
        self.assertEqual(is_sequence, True)

    def testShipPartsCoordinateInOnePlace(self):
        ship_parts_coordinates = [
            [1, 1],
            [1, 2]
        ]

        if len(ship_parts_coordinates) == 1:
            return
        
        x_in_one_place, y_in_one_place = self.coordinatesInOnePlace(ship_parts_coordinates)
        self.assertEqual(x_in_one_place + y_in_one_place, 1)
        ship_parts_coordinates = [[coords[1], coords[0]] for coords in ship_parts_coordinates]
        x_in_one_place, y_in_one_place = self.coordinatesInOnePlace(ship_parts_coordinates)
        self.assertEqual(x_in_one_place + y_in_one_place, 1)


class FriendRequestTest(TestCase):
    def testSameUsers(self):
        user = User.objects.create(
            user_name="username",
            user_password="user_password",
            user_id=1
        )

        friend_request = FriendRequest(
            from_user=user,
            to_user=user
        )
        
        self.assertRaises(ValidationError, friend_request.full_clean)

    def testInvalidType(self):
        with self.assertRaises(ValueError):
            FriendRequest.objects.create(from_user="")


class GameTest(TestCase):
    async def testDummy(self):
        game = await sync_to_async(Game.objects.create)(
            game_id="12951925u9125asf",
            user_id_turn=1
        )

        user = await sync_to_async(User.objects.create)(
            user_id=1,
            user_name="username",
            user_password="password",
            user_email="bot@bot.ru"
        )

        await sync_to_async(Field.objects.create)(
            owner=user,
            game=game
        )

        game_ids = await FieldDatabaseAccessor.getFieldsParents()
        waiting_games_id = list()

        viewed_games = list()
        async for idx, game_id in a.enumerate(game_ids):
            _game_id = game_id[0]
            if _game_id not in game_ids[:idx] + game_ids[idx + 1:] \
                and _game_id not in viewed_games:
                waiting_games_id.append(
                    (await sync_to_async(Game.objects.get)(id=_game_id)).game_id
                )
            viewed_games.append(
                (await sync_to_async(Game.objects.get)(id=_game_id)).game_id
            )

        self.assertEqual(len(waiting_games_id), 1)


class DummyTest(TestCase):
    async def testDummyTwo(self):
        game = await sync_to_async(Game.objects.create)(
            game_id="12951925u9125asf",
            user_id_turn=1
        )

        user = await sync_to_async(User.objects.create)(
            user_id=1,
            user_name="username",
            user_password="password",
            user_email="bot@bot.ru"
        )

        field = await sync_to_async(Field.objects.create)(
            owner=user,
            game=game
        )
        ship = await sync_to_async(Ship.objects.create)(
            field=field,
            ship_length=2
        )
        for x in range(2):
            await sync_to_async(ShipPart.objects.create)(
                ship=ship,
                x_pos=x,
                y_pos=3,
                is_damaged=False
            )
        for x in range(1, 2):
            for y in range(2, 3):
                try:
                    ship_part = await sync_to_async(ShipPart.objects.get)(
                        ship__field=field,
                        x_pos=x,
                        y_pos=y,
                        is_damaged=False
                    )
                    ship_part.is_damaged = True
                    await sync_to_async(ship_part.save)()

                    ship: Ship = await sync_to_async(getattr)(ship_part, "ship")
                    alive_ship_parts = await sync_to_async(ShipPart.objects.filter)(
                        ship=ship,
                        is_damaged=False
                    )
                    await sync_to_async(print)("ALIVE SHIP PARTS: ", alive_ship_parts)
                except ShipPart.DoesNotExist:
                    continue
                
                self.assertGreater(await sync_to_async(len)(alive_ship_parts), 0)


    async def testDummy(self):
        game = await sync_to_async(Game.objects.create)(
            game_id="12951925u9125asf",
            user_id_turn=1
        )

        user = await sync_to_async(User.objects.create)(
            user_id=1,
            user_name="username",
            user_password="password",
            user_email="bot@bot.ru"
        )

        field = await sync_to_async(Field.objects.create)(
            owner=user,
            game=game
        )
        ship = await sync_to_async(Ship.objects.create)(
            field=field,
            ship_length=5
        )
        for x in range(5):
            await sync_to_async(ShipPart.objects.create)(
                ship=ship,
                x_pos=x,
                y_pos=0,
                is_damaged=False
            )
        
        created_ship_parts = await sync_to_async(ShipPart.objects.filter)(
            ship=ship,
            is_damaged=False
        )

        self.assertEqual(await sync_to_async(len)(created_ship_parts), 5)

        await sync_to_async((await sync_to_async(ShipPart.objects.filter)(
            x_pos=2,
            y_pos=0,
        )).update)(is_damaged=True)

        dead_ship_parts = await sync_to_async(ShipPart.objects.filter)(
            ship__field=field,
            is_damaged=True
        )
    
        self.assertEqual(await sync_to_async(len)(dead_ship_parts), 1)

        alive_ship_parts = await sync_to_async(ShipPart.objects.filter)(
            ship=ship,
            is_damaged=False
        )

        self.assertEqual(await sync_to_async(len)(alive_ship_parts), 4)

        if (await sync_to_async(len)(alive_ship_parts)) == 0:
            ship.is_dead = True
            await sync_to_async(ship.save)()

            dead_ship_parts = await sync_to_async(ShipPart.objects.filter)(
                ship=ship
            )
            self.assertEqual(await sync_to_async(dead_ship_parts), 5)


class WeaponsTest(TestCase):
    async def testAccessing(self):
        weapon_types = await sync_to_async(WeaponType.objects.all)()
        async for weapon_type in weapon_types:
            print(weapon_type.weapon_type_name)

    def testCreating(self):
        for type in WeaponType.WeaponTypesNames:
            weapon_type = WeaponType(
                weapon_type_name=type,
                weapon_x_range=1,
                weapon_y_range=1,
                weapon_price=50.0
            )
            weapon_type.full_clean()
            weapon_type.save()
