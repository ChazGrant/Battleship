import requests


response = requests.post("http://127.0.0.1:8000/ships/delete_ships/")
# response = requests.post("http://127.0.0.1:8000/games/fire/", data={
#  	"game_id": "616943948676791553172445",
#  	"user_id": "ldutdhkyebvrdtj",
#  	"x": 1,
#  	"y": 5
#  	})
# response = requests.post("http://127.0.0.1:8000/games/create_game/", data={
#  	"user_id": "[gH[NAQ6C9bEe)t.JVLw"
#  	})
# response = requests.post("http://127.0.0.1:8000/games/game_is_over/", data={
#  	"game_id": "T%JwXr!x,IX*I<nip|^v"
#  	})
# response = requests.post("http://127.0.0.1:8000/games/fire/", data={
#   	"game_id": "212884236822661221883918",
#   	"user_id": "dfnyaucjhydlono",
#   	"x": 5,
#   	"y": 5
#   	})

# response = requests.post("http://127.0.0.1:8000/games/connect_to_game/", data={
#  	"user_id": "guest",
#  	"game_id": "i\\rD<a?w0e^oBKd<)ZD="
#  	})

# response = requests.post("http://127.0.0.1:8000/games/delete_games/")
print(response.json())