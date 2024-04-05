import os
from django.core.asgi import get_asgi_application # for the asgi application

from channels.routing import ProtocolTypeRouter, URLRouter
from WebsocketRequests.consumers import FriendlyDuelConsumer, FriendsUpdateConsumer

from django.urls import path   # provide the url path 

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')

ws_patterns= [
    path('friendly_duel/', FriendlyDuelConsumer.as_asgi()),
    path('friends_update/', FriendsUpdateConsumer.as_asgi())
]

get_asgi_application()

application= ProtocolTypeRouter({
    'websocket' : URLRouter(ws_patterns)
})
