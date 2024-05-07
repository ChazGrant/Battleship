from rest_framework.viewsets import ViewSet
from rest_framework.response import Response
from rest_framework.decorators import action

from django.db.models import Q
from django.db.utils import IntegrityError
from django.core.exceptions import ValidationError 

from random import randint

import json
import hashlib
from typing import Any, Dict

from .models import Game, Ship, User, FriendRequest, Friends, Weapon, WeaponType, PlayerLeague
from .serializers import GameSerializer, ShipSerializer, UserSerializer

from WebsocketRequests.JSON_RESPONSES import (
    USER_DOES_NOT_EXIST_JSON, NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON, 
    NOT_ENOUGH_WEAPONS_IN_STOCK, WEAPON_TYPE_DOES_NOT_EXIST_JSON, NOT_ENOUGH_COINS, 
    INVALID_WEAPONS_AMOUNT, INVALID_USER_NAME)

from typing import List

MAX_LIMIT = 1000


TRASNSLATED_COLUMN_NAMES = {
    "user_name": "Имя пользователя",
    "user_password": "Пароль",
    "user_email": "Электронная почта"
}

AVAILABLE_TOP_LENGTH = [3, 5, 10, 25]

# DEBUG
known_ips = list()


"""
    ViewSet кораблей

    Описывает поведение при запросе на адрес адрес_сервера/ships/имя_метода

    @author     ChazGrant
    @version    1.0
"""
class ShipViewSet(ViewSet):
    @action(detail=False, methods=["get"])
    def get_ships(self, request) -> Response:
        """
            Возвращает все корабли

            *DEBUG
            
            Возвращает:
                Сериализованный класс Ship
        """
        ships = Ship.objects.all()
        serialzer = ShipSerializer(ships, many=True)

        return Response(serialzer.data)


class GameViewSet(ViewSet):
    """
        ViewSet игры

        Описывает поведение при запросе на адрес адрес_сервера/games/имя_метода

        @author     ChazGrant
        @version    1.0
    """
    @action(detail=False, methods=["get"])
    def get_games(self, request) -> Response:
        """
            Возвращает все игры

            *DEBUG
        """
        queryset = Game.objects.all()
        serialzer = GameSerializer(queryset, many=True)

        return Response(serialzer.data)


class UserViewSet(ViewSet):
    """
        ViewSet пользователя

        Описывает поведение при запросе на адрес адрес_сервера/users/имя_метода

        @author     ChazGrant
        @version    1.0
    """
    def hashPassword(self, email: str, username: str, password: str) -> str:
        """
            Хэширует пароль

            Аргументы:
                email - Почта пользователя
                username - Имя пользователя
                password - Пароль
            
            Возвращает:
                Захэшированный пароль с солью
        """
        salt = ""

        for part in range(0, len(email), 2):
            salt += email[part]

        for part in range(0, len(username), 2):
            salt += username[part]
        
        return hashlib.md5((password + salt).encode()).hexdigest()

    @action(detail=False, methods=["get"])
    def create_test_users(self, request) -> Response:
        """
            Создаёт тестовых пользователей
            
            Возвращает:
                Созданных пользователей
        """
        for i in range(100):
            last_user_id = User.objects.all().last().user_id
            User.objects.create(
                user_name="test_user#" + str(i + 1),
                user_id=last_user_id+1,
                user_password="test_user_password",
                user_email="test_email#" + str(i + 1) + "@mail.ru",
                cups=randint(0, 300),
                silver_coins=randint(50, 350000),
                win_streak=randint(0, 10)
            )

        serializer = UserSerializer(User.objects.all(), many=True)
        return Response(serializer.data)

    @action(detail=False, methods=["post"])
    def login(self, request) -> Response:
        """
            Входит в аккаунт пользователя с заданными данными

            Аргументы:
                user_id - Идентификатор пользователя
                password - Незашифрованный пароль пользователя

            Возвращает:
                Текст ошибки и результат о логине или идентификатор пользователя
        """
        try:
            user_id = int(request.data["user_id"])
            password = request.data["password"]
        except ValueError:
            return Response(INVALID_ARGUMENTS_TYPE_JSON)
        except KeyError:
            return Response(NOT_ENOUGH_ARGUMENTS_JSON) 

        try:
            user = User.objects.get(user_id=user_id)
            hashed_password = self.hashPassword(user.user_email, user.user_name, password)
            User.objects.get(user_id=user_id, user_password=hashed_password)
        except User.DoesNotExist:
            return Response({
                "login_successful": False
            })

        return Response({
                "login_successful": True,
                "user_id": user.user_id,
                "user_name": user.user_name
            })

    @action(detail=False, methods=["post"])
    def registrate(self, request) -> Response:
        """
            Регистрирует пользователя

            Аргументы:
                user_name - Имя пользователя
                password - Пароль
                email - Электронная почта

            Возвращает:
                Информацию об успешной регистрации или текст ошибки
        """
        # x_forwarded_for = request.META.get("HTTP_X_FORWARDED_FOR")
        # if x_forwarded_for:
        #     ip = x_forwarded_for.split(",")[0]
        # else:
        #     ip = request.META.get("REMOTE_ADDR")

        # if ip in known_ips:
        #     return Response({
        #         "registration_successful": False
        #     })

        # known_ips.append(ip)
        try:
            user_name: str = request.data["user_name"]
            password: str = request.data["password"]
            email: str = request.data["email"]
        except KeyError:
            return Response({
                "registration_successful": False,
                "error": "Недостаточно параметров"
            })
        
        if user_name.lower().startswith("bot"):
            return Response(INVALID_USER_NAME)

        try:
            last_user_id = User.objects.latest("user_id").user_id
        except User.DoesNotExist:
            last_user_id = 0

        try:
            hashed_password = self.hashPassword(email, user_name, password)
            created_user = User(user_name=user_name, 
                                user_password=password, 
                                user_email=email,
                                user_id=last_user_id + 1)
            created_user.full_clean()
            created_user.user_password = hashed_password
            created_user.save()
        except IntegrityError as e:
            if "user_name" in e.args[0]:
                error = "Данное имя пользователя занято"
            elif "user_email" in e.args[0]:
                error = "Данная почта занята"
            return Response({
                "registration_successful": False,
                "error": error
            })
        except ValidationError as e:
            str_error = str(e).replace("'", "\"")
            error: dict = json.loads(str(str_error))
            error_message = "Неверно введены следующие поля:\n" + "\n".join([
                TRASNSLATED_COLUMN_NAMES[column_name] for column_name in error.keys()]
            )

            return Response({
                "registration_successful": False,
                "error": error_message
            })

        return Response({
            "registration_successful": True,
            "user_id": created_user.user_id,
            "user_name": created_user.user_name
        })


class FriendsViewSet(ViewSet):
    """
        ViewSet друзей

        Описывает поведение при запросе на адрес адрес_сервера/friends/имя_метода

        @author     ChazGrant
        @version    1.0
    """
    @action(detail=False, methods=["get"])
    def get_incoming_friend_requests(self, request) -> Response:
        """
            Получает входящие запросы в друзья

            Аргументы:
                user_id - Идентификатор пользователя, который хочет получить входящие заявки
            
            Возвращает:
                Список имён пользователей, которые отправили запрос на друзья
        """
        try:
            user_id = int(request.data["user_id"])
        except KeyError:
            return Response({
                "error": "Недостаточно параметров"
            })
        
        friend_requests: List[str] = []
        for friend_request in FriendRequest.objects.filter(to_user__user_id=user_id):
            friend_requests.append(friend_request.from_user.user_name)

        return Response({
            "friend_requests": friend_requests
        })

    @action(detail=False, methods=["get"])
    def get_friends(self, request) -> Response:
        """
            Получает друзей

            Аргументы:
                user_id - Идентификатор пользователя, который хочет получить своих друзей
            
            Возвращает:
                Список имён пользователей, которые в друзьях у запрошенного пользователя
        """    
        try:
            user_id = request.data["user_id"]
        except KeyError:
            return Response({
                "error": "Недостаточно параметров"
            })
        
        friends_names: List[str] = []
        try:
            user = User.objects.get(user_id=user_id)
        except User.DoesNotExist:
            return Response({
                "error": "Пользователя с указанным идентификатором не существует"
            })
        
        user_friends = Friends.objects.filter(Q(first_friend=user) | Q(second_friend=user))
        for user_friend in user_friends:
            if user_friend.second_friend.user_name == user.user_name:
                friends_names.append(user_friend.first_friend.user_name)
            else:
                friends_names.append(user_friend.second_friend.user_name)

        return Response({
            "friends": friends_names
        })


class LegaueViewSet(ViewSet):
    @action(detail=False, methods=["post"])
    def get_top_players(self, request) -> Response:
        players_by_cups: Dict[str, List[Dict[str, int]]] = {}
        players_by_silver_coins: Dict[str, List[Dict[str, int]]] = {}
        players_by_winstreak: Dict[str, List[Dict[str, int]]] = {}
        leagues: Dict[str, List[int]] = {}
        
        for league in PlayerLeague.objects.all():
            leagues[league.league_name] = [league.min_cups_required, league.max_cups_required]
            players_by_cups[league.league_name] = []
            players_by_silver_coins[league.league_name] = []
            players_by_winstreak[league.league_name] = []
        
        for league_name, (min_cups, max_cups) in leagues.items():
            users = User.objects.filter(
                is_temporary=False,
                cups__gte=min_cups,
                cups__lt=max_cups)
            players_by_cups[league_name] = []
            players_by_winstreak[league_name] = []
            players_by_silver_coins[league_name] = []
            for user in users.order_by("-cups"):
                players_by_cups[league_name].append({
                    "user_name": user.user_name,
                    "value": user.cups
                })

            for user in users.order_by("-win_streak"):
                players_by_winstreak[league_name].append({
                    "user_name": user.user_name,
                    "value": user.win_streak
                })

            for user in users.order_by("-silver_coins"):
                players_by_silver_coins[league_name].append({
                    "user_name": user.user_name,
                    "value": user.silver_coins
                })

        return Response({
            "leagues": list(leagues.keys()),
            "players_by_cups": players_by_cups,
            "players_by_silver_coins": players_by_silver_coins,
            "player_by_winstreak": players_by_winstreak,
        })


class ShopViewSet(ViewSet):
    """
        ViewSet магазина оружий

        Описывает поведение при запросе на адрес адрес_сервера/shop/имя_метода

        @author     ChazGrant
        @version    1.0
    """
    @action(detail=False, methods=["post"])
    def buy_weapon(self, request):
        try:
            user_id = int(request.data["user_id"])
            weapon_name = request.data["weapon_name"]
            buy_all = int(request.data["buy_all"])
            weapons_amount_to_buy = int(request.data["weapons_amount"])
        except KeyError:
            return Response(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return Response(INVALID_ARGUMENTS_TYPE_JSON)

        try:
            user = User.objects.get(user_id=user_id)
        except User.DoesNotExist:
            return Response(USER_DOES_NOT_EXIST_JSON)
        
        try:
            weapon_type = WeaponType.objects.get(weapon_type_name=weapon_name)
        except WeaponType.DoesNotExist:
            return Response(WEAPON_TYPE_DOES_NOT_EXIST_JSON)

        user.silver_coins = 50000
        user.save()
        weapon_price = weapon_type.weapon_price
        if buy_all:
            weapons_amount_to_buy = user.silver_coins // weapon_price

        total_price = weapon_price * weapons_amount_to_buy
        if weapons_amount_to_buy < 1:
            return Response(INVALID_WEAPONS_AMOUNT)

        if user.silver_coins < total_price:
            return Response(NOT_ENOUGH_COINS)

        try:
            user_weapons = Weapon.objects.get(
                weapon_owner=user,
                weapon_type=weapon_type
            )
        except Weapon.DoesNotExist:
            user_weapons = Weapon.objects.create(
                weapon_owner=user,
                weapon_type=weapon_type
            )

        user_weapons.weapon_amount += weapons_amount_to_buy
        user.silver_coins -= total_price
        
        user.save()
        user_weapons.save()

        return Response({
            "weapon_name": weapon_name,
            "weapons_amount": weapons_amount_to_buy,
            "weapon_sell_price": weapon_price * 0.75,
            "silver_coins_left": user.silver_coins
        })
    
    @action(detail=False, methods=["post"])
    def get_weapons(self, request):
        try:
            user_id = int(request.data["user_id"])
        except KeyError:
            return Response(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return Response(INVALID_ARGUMENTS_TYPE_JSON)
        
        try:
            user = User.objects.get(user_id=user_id)
        except User.DoesNotExist:
            return Response(USER_DOES_NOT_EXIST_JSON)
        
        data: Dict[str, List[Dict[str, Any]]] = dict()
        data["user_weapons"] = []
        data["all_weapons"] = []

        weapons = Weapon.objects.filter(weapon_owner=user)
        for weapon in weapons:
            data["user_weapons"].append({
                "weapon_name": weapon.weapon_type.weapon_type_name,
                "weapon_amount": weapon.weapon_amount,
                "weapon_sell_price": weapon.weapon_type.weapon_price * 0.75
            })
        weapon_types = WeaponType.objects.all()
        for weapon_type in weapon_types:
            data["all_weapons"].append({
                "weapon_name": weapon_type.weapon_type_name,
                "weapon_price": weapon_type.weapon_price,
                "weapon_x_range": weapon_type.weapon_x_range,
                "weapon_y_range": weapon_type.weapon_y_range
            })

        return Response(data)
        
    @action(detail=False, methods=["post"])
    def sell_weapon(self, request):
        try:
            user_id = int(request.data["user_id"])
            weapon_name = request.data["weapon_name"]
            sell_all = int(request.data["sell_all"])
            weapons_amount_to_sell = int(request.data["weapon_amount"])
        except KeyError:
            return Response(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return Response(INVALID_ARGUMENTS_TYPE_JSON)
        
        try:
            user = User.objects.get(user_id=user_id)
        except User.DoesNotExist:
            return Response(USER_DOES_NOT_EXIST_JSON)

        try:
            weapon_type = WeaponType.objects.get(weapon_type_name=weapon_name)
        except WeaponType.DoesNotExist:
            return Response(WEAPON_TYPE_DOES_NOT_EXIST_JSON)
        
        try:
            user_weapons = Weapon.objects.get(
                weapon_owner=user, weapon_type=weapon_type)
        except Weapon.DoesNotExist:
            return Response(NOT_ENOUGH_WEAPONS_IN_STOCK)
        
        if sell_all:
            weapons_amount_to_sell = user_weapons.weapon_amount

        user_weapons_amount = user_weapons.weapon_amount
        if weapons_amount_to_sell > user_weapons_amount:
            return Response(NOT_ENOUGH_WEAPONS_IN_STOCK)

        sold_weapon_price = weapon_type.weapon_price * 0.75
        user.silver_coins += sold_weapon_price * weapons_amount_to_sell
        user.save()

        user_weapons.weapon_amount = user_weapons_amount - weapons_amount_to_sell
        if user_weapons.weapon_amount == 0:
            user_weapons.delete()
        else:
            user_weapons.save()

        return Response({
            "weapon_name": weapon_name,
            "weapon_amount_left": user_weapons.weapon_amount,
            "silver_coins_left": user.silver_coins
        })

    @action(detail=False, methods=["post"])
    def get_user_coins(self, request):
        try:
            user_id = int(request.data["user_id"])
        except KeyError:
            return Response(NOT_ENOUGH_ARGUMENTS_JSON)
        except ValueError:
            return Response(INVALID_ARGUMENTS_TYPE_JSON)

        try:
            user = User.objects.get(user_id=user_id)
        except User.DoesNotExist:
            return Response(USER_DOES_NOT_EXIST_JSON)

        return Response({
            "silver_coins_left": user.silver_coins
        })
