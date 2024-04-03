from rest_framework import serializers

from .models import Game, Field, Ship, User, FriendRequest


class GameSerializer(serializers.ModelSerializer):
    class Meta:
        model = Game
        fields = ('game_id', 'user_id_turn', 'game_is_over')


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
