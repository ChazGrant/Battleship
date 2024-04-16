from rest_framework import serializers

from .models import Game, Field, Ship, User, FriendRequest, Friends, WeaponType


class GameSerializer(serializers.ModelSerializer):
    class Meta:
        model = Game
        fields = ('game_id', 'user_id_turn', 'game_is_over', 'is_friendly', 'game_invite_id')


class FieldSerializer(serializers.ModelSerializer):
    class Meta:
        model = Field
        fields = ('owner_id', 'four_deck', 'three_deck', 'two_deck', 'one_deck', 'game_id')


class ShipSerializer(serializers.ModelSerializer):
    class Meta:
        model = Ship
        fields = ('ship_length', 'shippart_set')


class UserSerializer(serializers.ModelSerializer):
    class Meta:
        model = User
        fields = ('user_id', 'user_name','user_password')


class FriendRequestSerializer(serializers.ModelSerializer):
    class Meta:
        model = FriendRequest
        fields = '__all__'


class FriendsSerializer(serializers.ModelSerializer):
    first_friend_id = serializers.CharField(source='first_friend.user_id')
    second_friend_id = serializers.CharField(source='second_friend.user_id')
    class Meta:
        model = Friends
        fields = ('first_friend_id', 'second_friend_id')


class WeaponTypeSerializer(serializers.ModelSerializer):
    class Meta:
        model = WeaponType
        fields = ('weapon_type_name', 'weapon_x_range', 'weapon_y_range', 'weapon_price')
