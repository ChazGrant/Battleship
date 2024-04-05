from channels.generic.websocket import AsyncJsonWebsocketConsumer

from typing import Dict

listeners:Dict[str, AsyncJsonWebsocketConsumer] = dict()


class EchoConsumer(AsyncJsonWebsocketConsumer):
    async def connect(self):
        await self.accept()

    async def receive(self, text_data):
        json_event = await self.decode_json(text_data)
        print("JSON EVENT: ", json_event)
        message_type = json_event["message_type"]
        if message_type == "subscribe":
            user_id = json_event["user_id"]
            listeners[user_id] = self

            await self.send(text_data=await self.encode_json({"status": "subscribed"}))
        elif message_type == "send_invite":
            from_user_id: str = json_event["from_user_id"]
            to_user_id: str = json_event["to_user_id"]
            game_id: str = json_event["game_id"]

            await listeners[to_user_id].send(text_data=await self.encode_json(
            {
                "type": "game_invite", 
                "game_id": str(game_id), 
                "from_user_id": str(from_user_id)
            }))
        elif message_type == "accept_invite":
            from_user_id: str = json_event["from_user_id"]
            to_user_id: str = json_event["to_user_id"]
            await listeners[to_user_id].send(text_data=await self.encode_json(
            {
                "type": "game_accept",
                "from_user_id":str(from_user_id)
            }))
            await self.send(text_data=await self.encode_json(
            {
                "status": "accepted"
            }))

    async def disconnect(self, event):
        print("Disconnected from server")
