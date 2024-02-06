from rest_framework.viewsets import ViewSet
from rest_framework.response import Response
from rest_framework.decorators import action
from django.db.models import F

import random

from .models import Game, Field, Ship, ShipPart, MarkedCell, User
from .serializers import GameSerializer, FieldSerializer, ShipSerializer

MAX_LIMIT = 1000

SHIP_LENGTHS = {
    1: lambda field: field.one_deck,
    2: lambda field: field.two_deck,
    3: lambda field: field.three_deck,
    4: lambda field: field.four_deck
}

SHIP_LENGTHS_NAMES = {
    1: 'one_deck',
    2: 'two_deck',
    3: 'three_deck',
    4: 'four_deck'
}

# DEBUG
known_ips = list()

# TODO
"""
    Впервые произошла полностью рабочая игра, в которой показали победителя и проигравшего

    Теперь нужно искать баги, один из которых является багом Шрёддингера:
        Иногда, зачастую во время первых ходов, игроку не отрисовываются те корабли, по которым попал противник
    Ещё один баг заключается в том, что при создании серых частей вокруг мёрвтого корабля, условие пропускается
    из-за чего создаётся серая часть в месте, где находится мёртвая часть корабля
"""

"""
    1) Игрок подключается к игре, получая её id
    2) Игрок ставит все свои корабли и ожидает пока не подключиться другой игрок или другой игрок не выставит
    все корабли
    3) Игрок получает информацию о том, чей сейчас ход
    4) Игрок стреляет по кораблям, при промахе ход меняется, при попадании сервер возвращает поля,
    по которым попал игрок
    5) Второй игрок получает информацию о том, по каким клеткам попал первый игрок
    6) Когда все корабли уничтожены, игрок получает информацию об этом и на основе этого узнаёт кто победил
    7) Игра заканчивается и удаляется вместе с полями, кораблями и частями кораблей при следующих условиях:
        - Один из игроков покинул игру, т.е. закрыл окно с игрой
        - Корабли одного из игроков были полностью уничтожены
        - Один из игроков не сделал ничего в течении минуты, из-за чего был кикнут с игры
"""


def createMarkedCellsAroundShip(ship: Ship, field: Field) -> list:
    missed_cells = list()
    dead_ship_parts = ShipPart.objects.filter(ship=ship)
    for dead_part in dead_ship_parts:
        x_pos = dead_part.x_pos
        y_pos = dead_part.y_pos

        # Проходимся по квадрату вокруг мёртвой клетки
        for x in range(x_pos - 1, x_pos + 2):
            for y in range(y_pos - 1, y_pos + 2):
                # Клетка за пределами поля
                if x < 0 or y < 0 or x > 9 or y > 9:
                    continue
                
                # Если координаты совпадают с координатами части корабля, то часть не является missed
                if x_pos == x and y_pos == y:
                    continue

                # Добавляем 'серую' клетку
                missed_cells.append(f"{x} {y}")
                # Создаём MarkedCell, если такой ещё нет
                try:
                    MarkedCell.objects.get(field=field, x_pos=x, y_pos=y)
                except MarkedCell.DoesNotExist:
                    MarkedCell.objects.create(field=field, x_pos=x, y_pos=y)

    return missed_cells

def generateGameId():
    return "".join(str(random.randint(0, MAX_LIMIT)) for i in range(8))

def allShipsHasBeenPlaced(field: Field) -> bool:
    return (field.four_deck + field.three_deck + field.two_deck + field.one_deck) == 0

def getDamagedShipPartsPositions(ship: Ship) -> list:
    parts_positions = list()
    damaged_ship_parts = ShipPart.objects.filter(ship=ship, is_damaged=True)

    for damaged_ship_part in damaged_ship_parts:
        # Делаем так, потому что в Qt легче обработать массив из строк(через split каждого элемента)
        parts_positions.append(f"{damaged_ship_part.x_pos} {damaged_ship_part.y_pos}")

    return parts_positions

def shipIsDead(ship: Ship) -> bool:
    ship_parts = ShipPart.objects.filter(ship=ship)
    return all(map(lambda part: part.is_damaged, ship_parts))

def allShipsAreDead(field: Field) -> bool:
    ships = Ship.objects.filter(field=field)
    # Если все корабли мертвы, то возвращает True
    return all(map(lambda ship: ship.is_dead, ships))

def getWinner(game: Game) -> str:
    # Если у игры уже есть победитель, значит второй игрок закончил игру
    # и его поле удалено
    if game.has_winner:
        # Возвращаем любую строку, т.к. id пользователя с этой строкой точно совпадать
        # не будет, а соответственно клиент будет считать себя проигравшим
        return "not you"

    # Если у 0 поля все корабли погибли, то победило 1 поле
    fields = Field.objects.filter(game=game)

    for idx, field in enumerate(fields):
        if allShipsAreDead(field):
            swapGameUserIdTurn(game=game)
            # Заканчиваем игру
            game.game_is_over = True
            game.has_winner = True
            game.save()

            return fields[not idx].owner_id
    return ""

def swapGameUserIdTurn(game: Game) -> None:
    fields = Field.objects.filter(game=game)
    game_id = game.game_id

    owners_id = [field.owner_id for field in fields]

    # Получаем индекс id игрока, которого сейчас ход
    user_id_turn = owners_id.index(game.user_id_turn)

    game.user_id_turn = owners_id[not user_id_turn]
    game.save()


class ShipViewSet(ViewSet):
    @action(detail=False, methods=['post'])
    def return_ships(self, request) -> Response:
        ships = Ship.objects.all()
        serialzer = ShipSerializer(ships, many=True)

        return Response(serialzer.data)

    @action(detail=False, methods=['post'])
    def delete_ships(self, request) -> Response:
        Ship.objects.all().delete()
        Field.objects.all().delete()
        Game.objects.all().delete()

        return Response({
            "deleted": True
        })


class FieldViewSet(ViewSet):
    def createShip(self, cells: list, field: Field) -> None:
        ship_length = len(cells)
        ship = Ship.objects.create(field=field, ship_length=ship_length)
        for cell in cells:
            x, y = cell
            ShipPart.objects.create(ship=ship, x_pos=x, y_pos=y)

        ship_length_str = SHIP_LENGTHS_NAMES[ship_length]

        # Это был просто эксперимент, было интересно найти способ передавать в update
        # поле которое нужно изменить и его новое значение
        # В итоге передаётся обычный словарь kwargs, в которой указано название поля и его новое значение
        kwargs = {
            ship_length_str: F(ship_length_str) - 1
        }
        Field.objects.filter(owner_id=field.owner_id).update(**kwargs)

    def hasCollisions(self, ships: list, cells: list) -> bool:
        # Закэшировать, т.к. объёмные вычисления
        marked_cells = list()
        # Поиск клеток, заполненных кораблями
        for ship in ships:
            ship_parts = ShipPart.objects.filter(ship=ship)
            for ship_part in ship_parts:
                for cell in cells:
                    cell_x, cell_y = cell

                    if ship_part.x_pos in [cell_x - 1, cell_x + 1, cell_x] \
                    and ship_part.y_pos in [cell_y - 1, cell_y + 1, cell_y]:
                        return True

        return False

    # 5)
    @action(detail=False, methods=['post'])
    def get_damaged_cells(self, request) -> Response:
        '''
            user_id
        '''            
        try:
            user_id = request.data['user_id']
        except KeyError:
            return Response({
                'Error': 'Not enough arguments'
            })

        field = Field.objects.get(owner_id=user_id)

        data = dict()
        # Получаем все погибшие корабли
        dead_ships = Ship.objects.filter(field=field, is_dead=True)
        dead_parts = list()
        for dead_ship in dead_ships:
            for dead_part in getDamagedShipPartsPositions(dead_ship):
                dead_parts.append(dead_part)

        dead_parts = [dead_part for dead_part in dead_parts if dead_part]
        data["dead_parts"] = dead_parts

        # Получаем все корабли
        ships = Ship.objects.filter(field=field, is_dead=False)
        damaged_parts = list()
        for ship in ships:
            for damaged_part in getDamagedShipPartsPositions(ship):
                damaged_parts.append(damaged_part)

        damaged_parts = [damaged_part for damaged_part in damaged_parts if damaged_part]
        data["damaged_parts"] = damaged_parts

        # Получаем все повреждённые клетки
        marked_cells = MarkedCell.objects.filter(field=field)
        if marked_cells:
            marked_cells = [f"{cell.x_pos} {cell.y_pos}" for cell in marked_cells]
            data["marked_cells"] = marked_cells
        
        return Response(data)

    # 2)
    @action(detail=False, methods=['post', 'get'])
    def return_field(self, request) -> Response:
        if 'owner_id' not in request.data:
            field = Field.objects.all()
            serializer = FieldSerializer(field, many=True)
            return Response(serializer.data)
            return Response(
            {
                "Error": "Not enough arguments"
            })

        try:
            field = Field.objects.get(owner_id=request.data["owner_id"])
        except Field.DoesNotExist:
            return Response(
            {
                "Critical Error": "Field does not exist"
            })

        serialzer = FieldSerializer(field)
        return Response(serialzer.data)

    # 2)
    @action(detail=False, methods=['post'])
    def place_ship(self, request) -> Response:
        '''
            owner_id, game_id
        '''
        cells = list()
        data = dict()            
        try:
            owner_id = request.data['owner_id']
            game_id = request.data['game_id']
            # [[5, 7], [5, 8], [5, 9]]
            cells = request.data['cells'].split(" ")
        except KeyError:
            return Response(
            {
                'Error': 'Not enough arguments'
            })

        cells = list(map(lambda lst: lst.split(","), cells))
        cells = [cell for cell in cells if any(cell)]
        cells = [list(map(int, cell)) for cell in cells]

        # Проверка на выход за поле
        for cell in cells:
            x, y = cell
            if x < 0 or x > 9 or y < 0 or y > 9:
                return Response({
                    "Error": "Out of bounds"
                })

        if not cells:
            return Response({
                "Error": "No ships to place"
            })

        # Проверка ширины корабля
        first_cell = cells[0]
        for cell in cells:
            if cell[0] != first_cell[0] and cell[1] != first_cell[1]:
                return Response({
                    "Error": "Ship is too wide:)"
                })
        data["cells"] = cells

        # Проверка длины корабля
        ship_length = len(cells)
        if ship_length > 4:
            return Response({
                "Error": "Ship is too long"
            })
        field = Field.objects.get(owner_id=owner_id)

        # Проверка осталось ли столько кораблей
        if not SHIP_LENGTHS[ship_length](field=field):
            return Response(
            {
                "Error": f"There no {ship_length}-deck ships last"
            })

        # Получаем все корабли на поле
        ships = Ship.objects.filter(field=field)

        # Если кораблей нет, то нет и коллизий
        if not ships:
            self.createShip(cells=cells, field=field)

            return Response(data)
        else:
            if (self.hasCollisions(ships=ships, cells=cells)):
                return Response(
                {
                    "Error": "Collisions"
                })
            else:
                self.createShip(cells=cells, field=field)

        # Если все корабли заполнены - вернуть в ответе
        if allShipsHasBeenPlaced(field=field):
            data["all_ships_has_been_placed"] = True
 
        return Response(data)


class GameViewSet(ViewSet):
    def deleteGame(self, game_id: str) -> None:
        Game.objects.filter(game_id=game_id).delete()

    @action(detail=False, methods=['post'])
    def disconnect(self, request) -> Response:
        '''
            user_id, game_id
        '''          
        try:
            game_id = request.data['game_id']
            user_id = request.data['user_id']
        except KeyError:
            return Response(
            {
                "Error": "Not enough arguments"
            })

        try:
            game = Game.objects.get(game_id=game_id)
        except Game.DoesNotExist:
            return Response(
            {
                "Critical Error": "Game doesn't exist"
            })

        try:
            Field.objects.filter(game=game, owner_id=user_id).delete()
        except Field.DoesNotExist:
            return Response(
            {
                "Critical Error": "Field doesn't exist"
            })

        # Если полей у это игры нет, значит оба игрока вышли и игру можно удалять
        try:
            Field.objects.get(game=game)
        except Field.DoesNotExist:
            Game.objects.filter(game_id=game_id).delete()

        return Response(
        {
            "Disconnected": True
        })

    # 6)
    @action(detail=False, methods=['post'])
    def game_is_over(self, request) -> Response:
        '''
            game_id, user_id
        '''
        # Если игра окончена, значит поля можно удалять
        # сначала удаляется поле победителя, затем второй игрок удаляет своё поле

        # У игры 2 состояния
        # 1: Игра окончена, но победителя нет
        # 2: Игра окончена, и победитель есть

        game_id = request.data['game_id']
        try:
            game = Game.objects.get(game_id=game_id)
        except Game.DoesNotExist:
            return Response(
            {
                "Critical Error": "Game does not exist"
            })

        data = dict()
        user_id = request.data['user_id']
        winner = getWinner(game=game)

        if (winner):
            data["winner"] = winner

            # Удаляем поле, т.к. у игрока игра закрывается
            Field.objects.filter(game=game, owner_id=user_id).delete()
        
        data["game_is_over"] = game.game_is_over

        # Если полей нет, значит все вышли и игру можно удалять
        if (len(Field.objects.filter(game=game)) == 0):
            self.deleteGame(game_id=game_id)
        return Response(data)

    # 3)
    @action(detail=False, methods=['post'])
    def get_user_id_turn(self, request) -> Response:
        '''
            game_id
        '''
        game_id = request.data['game_id']
        try:
            game = Game.objects.get(game_id=game_id)
        except Game.DoesNotExist:
            # Если игры не существует, то она не была создана
            return Response(
            {
                "Critical Error": "Game doesn't exist"
            })


        # Если у игры не 2 поля, значит второй игрок закрыл игру
        fields = Field.objects.filter(game=game)

        # Если поле 1 и у игры есть победитель, значит победил второй игрок
        # Иначе второй игрок вышел
        if len(fields) < 2 and not game.has_winner:
            return Response(
            {
                "Critical Error": "Current game has less than 2 fields"
            })

        print(game.user_id_turn)
        return Response(
        {
            "user_id_turn": game.user_id_turn
        })

    # 2)
    @action(detail=False, methods=['post'])
    def game_is_started(self, request) -> Response:
        '''
            game_id
        '''
        # Если корабли у обеих полей закончены то игра начата
        game_id = request.data['game_id']
        game = Game.objects.get(game_id=game_id)

        fields = Field.objects.filter(game=game)

        if len(fields) != 2:
            return Response(
            {
                "game_is_started": False
            })

        if (allShipsHasBeenPlaced(fields[0]) and allShipsHasBeenPlaced(fields[1])):
            return Response(
            {
                "game_is_started": True
            })

        return Response(
        {
            "game_is_started": False
        })

    # 1)
    @action(detail=False, methods=['post'])
    def create_game(self, request) -> Response:
        try:  
            user_id = request.data['user_id']
        except KeyError:
            return Response(
            {
                "Error": "Not enough arguments"
            })

        try:
            field = Field.objects.get(owner_id=user_id)

            return Response(
            {
                "Error": "User already has a game"
            })
        except Field.DoesNotExist:
            game_id = generateGameId()
            game = Game.objects.create(game_id=game_id, user_id_turn=user_id)
            field = Field.objects.create(owner_id=user_id, game=game)

            game_serializer = GameSerializer(game)
            return Response(game_serializer.data)

    # 4)
    @action(detail=False, methods=['post'])
    def fire(self, request) -> Response:
        """
            game_id, user_id, x, y
        """
        ### Если убивает последний корабль, то возвращаем информацию о том, что игра закончена ###

        # Получаем игру, в которой находится игрок и его айди, а также координаты, по которым он стреляет
        try:
            game_id = request.data['game_id']
            user_id = request.data["user_id"]
            x = int(request.data["x"])
            y = int(request.data["y"])
        except KeyError:
            return Response(
            {
                "Error": "Not enough arguments"
            })

        # Получаем игру, в которой находится пользователь
        try:
            game = Game.objects.get(game_id=game_id)
        except Game.DoesNotExist:
            return Response(
            {
                "Critical Error": "Game does not exist"
            })


        # Если ход не его, то стрелять он не может
        if not game.user_id_turn == user_id:
            return Response(
            {
                "Error": "It's not your turn"
            })


        # Получаем поле противника
        # путём исключения из полей поля с id пользователя
        field = Field.objects.filter(game=game).exclude(owner_id=user_id)

        # Обработка, при нахождении более одного поля
        if len(field) > 1:
            return Response(
            {
                "Critical Error": "Found more than one player's field"
            })
        field = field[0]
        # Получаем координаты, по которым стреляет игрок
        

        # Получаем все корабли противника
        ships = Ship.objects.filter(field=field)
        data = dict()

        # Проходимся по кораблям
        for ship in ships:
            try:
                needed_ship_part = ShipPart.objects.get(ship=ship, x_pos=x, y_pos=y)

                if needed_ship_part.is_damaged:
                    return Response(
                    {
                        "Error": "Ship part is already damaged"
                    })

                needed_ship_part.is_damaged = True
                needed_ship_part.save()

                # Проверяем все ли части корабля повреждены
                ship_is_dead = shipIsDead(ship)

                # Если корабль мёртв, то возвращаем его части и всю область вокруг него
                if ship_is_dead:
                    # Делаем его мёртвым
                    ship.is_dead = True
                    ship.save()


                    ship_parts = ShipPart.objects.filter(ship=ship)
                    # Мёртвые части
                    dead_parts = [f"{part.x_pos} {part.y_pos}" for part in ship_parts]

                    missed_cells = createMarkedCellsAroundShip(ship=ship, field=field)

                    return Response(
                    {
                        "ship_is_killed": True,
                        "dead_parts": dead_parts,
                        "missed_cells": missed_cells,
                    })
                # Иначе возвращаем повреждённую часть корабля
                else:
                    return Response(
                        {
                            "ship_is_damaged": True,
                            "damaged_part": f"{x} {y}"
                        })       
            # Если части нет, значит не попали
            except ShipPart.DoesNotExist:
                pass

        # Нужно проверить, стреляли по этой клетке уже или нет
        try:
            # Ищем клетку в базе
            marked_cell = MarkedCell.objects.get(field=field, x_pos=x, y_pos=y)

            # Если она в базе, значит по ней уже стреляли
            return Response(
            {
                "Error": "Cell is already damaged"
            })

        except MarkedCell.DoesNotExist:
            MarkedCell.objects.create(field=field, x_pos=x, y_pos=y)

            # Меняем ход игрока, т.к. попаданий не было
            swapGameUserIdTurn(game=game)

            return Response(
            {
                "missed": True,
                "missed_cell": f"{x} {y}"
            })

    @action(detail=False, methods=['get', 'post'])
    def return_games(self, request) -> Response:
        queryset = Game.objects.all()
        serialzer = GameSerializer(queryset, many=True)

        return Response(serialzer.data)

    # 1)
    @action(detail=False, methods=['post'])
    def connect_to_game(self, request) -> Response:
        """
            game_id, user_id
        """
        try:
            game_id = request.data['game_id']
            user_id = request.data['user_id']
        except KeyError:
            return Response(
            {
                "Error": "Not enough arguments"
            })

        try:
            game = Game.objects.get(game_id=game_id)
            fields = Field.objects.filter(game=game)
        except Game.DoesNotExist:
            return Response(
            {
                "Error": "Current game does not exist"
            })


        # В комнате уже 2 поля, соответственно 2 игрока
        if len(fields) == 2:
            return Response(
            {
                "Error": "Game is full"
            })

        # Проверка на то, что игрок с таким же id подключается
        if fields[0].owner_id == user_id:
            return Response(
            {
                "Error": "Game cannot contain players with 2 same id"
            })
        
        Field.objects.create(game=game, owner_id=user_id)

        return Response(
        {
            "game_id": game_id
        })

    @action(detail=False, methods=['post'])
    def delete_game(self, request) -> Response:     
        try:
            game_id = request.data["game_id"]
        except KeyError:
            return Response(
            {
                "Error": "Not enough arguments"
            })

        Game.objects.filter(game_id=game_id).delete()

        return Response(
        {
            "deleted": True
        })

    @action(detail=False, methods=['post'])
    def delete_games(self, request) -> Response:
        Game.objects.all().delete()

        return Response({
            "deleted": True
        })


class UserViewSet(ViewSet):
    @action(detail=False, methods=["post"])
    def login(self, request):
        try:
            user_id = int(request.data['user_id'])
        except ValueError:
            return Response({"login_successful": False})
        password = request.data['password']

        print(user_id)
        print(password)

        try:
            user = User.objects.get(user_id=user_id, user_password=password)
        except User.DoesNotExist:
            return Response({"login_successful": False})

        return Response({
                "login_successful": True,
                "username": user.user_name
            })

    @action(detail=False, methods=["post"])
    def registrate(self, request):
        x_forwarded_for = request.META.get('HTTP_X_FORWARDED_FOR')
        if x_forwarded_for:
            ip = x_forwarded_for.split(',')[0]
        else:
            ip = request.META.get('REMOTE_ADDR')
        
        if ip in known_ips:
            return Response({
                "registrate_successful": False
            })
        
        known_ips.append(ip)
        last_user = User.objects.get()
