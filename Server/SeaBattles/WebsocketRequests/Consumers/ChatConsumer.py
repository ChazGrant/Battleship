from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict, Coroutine, Callable

from WebsocketRequests.JSON_RESPONSES import (NOT_ENOUGH_ARGUMENTS_JSON, INVALID_ARGUMENTS_TYPE_JSON,
                            INVALID_ACTION_TYPE_JSON, USER_DOES_NOT_EXIST_JSON)

from WebsocketRequests.DatabaseAccessors.UserDatabaseAccessor import UserDatabaseAccessor
from WebsocketRequests.DatabaseAccessors.FriendRequestDatabaseAccessor import FriendRequestDatabaseAccessor


class ChatConsumer(AsyncJsonWebsocketConsumer):
    groups = []
    def __init__(self):
        self._available_actions: Dict[str, Callable] = {
            "send_message": self.sendMessage
        }

    async def sendMessage(self, json_object: dict) -> None:
        ...

    async def connect(self):
        ...

    async def disconnect(self, code):
        ...
    
    async def receive_json(self, json_object: dict) -> None:
        ...
