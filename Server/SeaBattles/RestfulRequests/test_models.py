from django.test import TestCase
from django.core.exceptions import ValidationError
from django.db.utils import IntegrityError

from RestfulRequests.models import User, Field, Game, ShipPart, FriendRequest


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
