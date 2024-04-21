from django.test import TestCase
from django.core.exceptions import ValidationError
from django.db.utils import IntegrityError

from asgiref.sync import sync_to_async

from RestfulRequests.models import User, Field, Game, ShipPart, FriendRequest, WeaponType, Weapon

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


class WeaponsTest(TestCase):
    async def testAccessing(self):
        for weapon_type_name in WeaponType.Types:
            print(weapon_type_name)
        weapon_types = await sync_to_async(WeaponType.objects.all)()
        async for weapon_type in weapon_types:
            print(weapon_type.weapon_type_name)

    async def testCreating(self):
        def initWeaponTypes():
            for type in WeaponType.Types:
                weapon_type = WeaponType(
                    weapon_type_name=type,
                    weapon_x_range=1,
                    weapon_y_range=1,
                    weapon_price=50.0
                )
                weapon_type.full_clean()
                weapon_type.save()

        await sync_to_async(initWeaponTypes)()

        created_user = await sync_to_async(User.objects.create)(
            user_name="bott",
            user_password="password",
            user_email="bott@bot.bot",
            user_id=1
        )
        single_shoot = await sync_to_async(WeaponType.objects.get)(weapon_type_name=WeaponType.Types.SINGLE_SHOOT)
        await sync_to_async(Weapon.objects.create)(
            weapon_owner=created_user,
            weapon_type=single_shoot,
            is_infinite=True,
            weapon_amount=999
        )

        weapons = await WeaponDatabaseAccessor.getAvailableWeapons(created_user.user_id)

        self.assertEqual(weapons["Пушечный выстрел"], 999)

