"""SeaBattles URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path, include

from rest_framework.routers import DefaultRouter

from Games.views import GameViewSet, FieldViewSet, ShipViewSet, UserViewSet, FriendsViewSet


router = DefaultRouter()
router.register(r'games', GameViewSet, basename='game')
router.register(r'fields', FieldViewSet, basename='field')
router.register(r'ships', ShipViewSet, basename='ship')
router.register(r'users', UserViewSet, basename='user')
router.register(r'friends', FriendsViewSet, basename='friends')

urlpatterns = [
    path('', include('rest_framework.urls')),
    path('', include(router.urls)),
    # path('games/', include('Games.urls'))
]
