from colorama import just_fix_windows_console, Fore
just_fix_windows_console()
import random
import os


def generateCoordinates(ship_length: int, field: int):
    def generateCells(orientation: str):
        cells = []
        if orientation == 'horizontal':
            x = random.randint(0, 9 - ship_length)
            y = random.randint(0, 9)
            for i in range(ship_length):
                cells.append([x + i, y])
        else:
            x = random.randint(0, 9)
            y = random.randint(0, 9 - ship_length)
            for i in range(ship_length):
                cells.append([x, y + i])
        
        return cells

    cells = []
    orientations = ['horizontal', 'vertical']
    orientation = random.choice(orientations)
    orientation_changed = False
    cells = generateCells(orientation)
    
    while True:
        if not hasCollisions(field, cells):
            ship = []
            for cell in cells:
                field[cell[0]][cell[1]] = 1
                ship.append([cell[1], cell[0]])
            ships.append(ship)
            return True
        else:
            if not orientation_changed:
                orientation = orientations[not orientations.index(orientation)]
                orientation_changed = True
            else:
                cells = generateCells(orientation)
                orientation_changed = False

def hasCollisions(field, cells):
    for cell in cells:
        cell_x, cell_y = cell
        for x in range(cell_x - 1, cell_x + 2):
            for y in range(cell_y - 1, cell_y + 2):
                _x, _y = x, y
                if x < 0:
                   _x = 0 
                if y < 0:
                    _y = 0
                try:
                    if field[_x][_y] == 1:
                        return True
                except IndexError:
                    pass

    return False

def initField():
    ships_amounts = {
        4: 1,
        3: 2,
        2: 3,
        1: 4
    }
    for ship_length, ship_amount in ships_amounts.items():
        ships_generated = 0
        i = 0
        while ships_generated < ship_amount:
            # if i > 250:
            #     print(ship_length)
            #     drawField()
            #     exit()
            # i += 1
            ships_generated += generateCoordinates(ship_length, field)
                
def shipIsDead(field, x, y):
    is_dead = False
    for ship in ships:
        if [x, y] in ship:
            for ship_x, ship_y in ship:
                if field[ship_y][ship_x] == -1:
                    is_dead = True
                else:
                    return False
            break
                
    return is_dead

def drawField():
    for row in field:
        for column in row:
            if column == 1:
                print(Fore.GREEN + "&", end=" ")
            elif column == -1:
                print(Fore.RED + "&", end=" ")
            elif column == -2:
                print(Fore.YELLOW + "&", end=" ")
            else:
                print(Fore.WHITE + "&", end=" ")
        print(Fore.WHITE)

def getNextPotentionalCell(field, damaged_ship_cells, damaged_cells):
    x_one_place = all(map(lambda x: x == damaged_ship_cells[0][1], 
                    [x[1] for x in damaged_ship_cells]))
    y_one_place = all(map(lambda y: y == damaged_ship_cells[0][0], 
                    [y[0] for y in damaged_ship_cells]))

    # Это значит что координата одна
    if x_one_place and y_one_place:
        potentional_ship_part_cells = [
            [damaged_ship_cells[0][0] + 1, damaged_ship_cells[0][1]],
            [damaged_ship_cells[0][0] - 1, damaged_ship_cells[0][1]],
            [damaged_ship_cells[0][0], damaged_ship_cells[0][1] + 1],
            [damaged_ship_cells[0][0], damaged_ship_cells[0][1] - 1]
        ]
        potentional_ship_part_cells = [cell for cell in potentional_ship_part_cells if 
                                       0 <= cell[0] < 10 and 0 <= cell[1] < 10]
        
        for potentional_ship_part_cell in potentional_ship_part_cells:
            if (potentional_ship_part_cell not in damaged_cells):
                return potentional_ship_part_cell
    else:
        potentional_ship_cells = []
        for damaged_ship_cell in damaged_ship_cells:
            potentional_ship_part_cells = [
                [damaged_ship_cell[0] + x_one_place, damaged_ship_cell[1] + y_one_place],
                [damaged_ship_cell[0] - x_one_place, damaged_ship_cell[1] - y_one_place],
            ]
            potentional_ship_part_cells = [cell for cell in potentional_ship_part_cells if 
                                       0 <= cell[0] < 10 and 0 <= cell[1] < 10]
            
            for potentional_ship_part_cell in potentional_ship_part_cells:
                potentional_x, potentional_y = potentional_ship_part_cell
                if (potentional_ship_part_cell not in potentional_ship_cells) and \
                    (potentional_ship_part_cell not in damaged_cells):
                    return [potentional_x, potentional_y]

def getShipsLeft(damaged_cells, ships):
    ships_left = {}
    for ship in ships:
        if len(ship) not in ships_left.keys():
            ships_left[len(ship)] = 0
        ship_is_alive = len([ship_part for ship_part in ship if not ship_part in damaged_cells])
        if ship_is_alive:
            ships_left[len(ship)] = ships_left[len(ship)] + 1

    return ships_left

if __name__ == "__main__":

    # for x in range(10):
    #     for y in range(10):
    #         if field[x][y] == 1:
    #             field[x][y] = -1
    # drawField()

    for i in range(5000001):
        if i % 5000 == 0:
            print(i)
        field = [[0 for _ in range(10)] for _ in range(10)]
        ships = []
        initField()
        damaged_cells = []
        damaged_ship_cells = []
        if not getShipsLeft(damaged_cells, ships) == {4: 1, 3: 2, 2: 3, 1: 4}:
            drawField()
            print(getShipsLeft(damaged_cells, ships))
            raise Exception()
    exit()
    while(any(map(lambda row: any(map(lambda column: column > 0, row)), field))):
        if damaged_ship_cells:
            x_pos, y_pos = getNextPotentionalCell(field, damaged_ship_cells, damaged_cells)
        else:
            x_pos = random.randint(0, 9)
            y_pos = random.randint(0, 9)
        if [x_pos, y_pos] not in damaged_cells:
            damaged_cells.append([x_pos, y_pos])
            item_value = field[y_pos][x_pos]
            if item_value == 1:
                field[y_pos][x_pos] = -1
                ship_is_dead = shipIsDead(field, x_pos, y_pos)
                if not ship_is_dead:
                    damaged_ship_cells.append([x_pos, y_pos])
                else:
                    for damaged_ship in damaged_ship_cells:
                        damaged_cells.append(damaged_ship)
                    damaged_ship_cells = []
            else:
                field[y_pos][x_pos] = -2
                damaged_cells.append([x_pos, y_pos])
            drawField()
            print("SHIPS LEFT: ")
            print(getShipsLeft(damaged_cells, ships))
            print()
            # input()
