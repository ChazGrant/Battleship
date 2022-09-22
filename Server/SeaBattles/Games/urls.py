from django.urls import path, include

from .views import create_game

urlpatterns = [
	path('create_game/<int:user_id>', create_game)
]