from rest_framework import serializers

from .models import Game, Field, Ship, User, FriendRequest, Friends, WeaponType, PlayerLeague


class GameSerializer(serializers.ModelSerializer):
    class Meta:
        model = Game
        fields = ('game_id', 'user_id_turn', 'game_is_over', 'is_friendly', 'game_invite_id')


class FieldSerializer(serializers.ModelSerializer):
    _owner_id = serializers.IntegerField(source='owner.user_id')
    class Meta:
        model = Field
        fields = ('owner', '_owner_id', 'four_deck', 'three_deck', 'two_deck', 'one_deck', 'game_id')


class ShipSerializer(serializers.ModelSerializer):
    class Meta:
        model = Ship
        fields = ('ship_length', 'shippart_set')


class UserSerializer(serializers.ModelSerializer):
    class Meta:
        model = User
        fields = '__all__'


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
        fields = ('__all__')


class LeagueSerializer(serializers.ModelSerializer):
    class Meta:
        model = PlayerLeague
        fields = ('__all__')
