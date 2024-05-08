"""SeaBattles URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path("", views.home, name="home")
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path("", Home.as_view(), name="home")
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path("blog/", include("blog.urls"))
"""
from django.urls import path, include

from rest_framework.routers import DefaultRouter

from RestfulRequests.views import (GameViewSet, ShopViewSet, ShipViewSet, UserViewSet, 
                                   FriendsViewSet, LegaueViewSet, FieldViewSet)


router = DefaultRouter()
router.register(r"games", GameViewSet, basename="game")
router.register(r"ships", ShipViewSet, basename="ship")
router.register(r"users", UserViewSet, basename="user")
router.register(r"friends", FriendsViewSet, basename="friends")
router.register(r"shop", ShopViewSet, basename="shop")
router.register(r"leagues", LegaueViewSet, basename="league")
router.register(r"fields", FieldViewSet, basename="field")

urlpatterns = [
    path("", include("rest_framework.urls")),
    path("", include(router.urls))
]
