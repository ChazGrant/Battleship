import os
from channels.routing import ProtocolTypeRouter, URLRouter
from django.core.asgi import get_asgi_application # for the asgi application
from django.urls import path   # provide the url path 
from websocket.consumers import * # it is used for the applications views

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'SeaBattles.settings')

application = get_asgi_application()

ws_patterns= [
        
    path('ws/',EchoConsumer.as_asgi())
]

application= ProtocolTypeRouter({

    'websocket' : URLRouter(ws_patterns)


})