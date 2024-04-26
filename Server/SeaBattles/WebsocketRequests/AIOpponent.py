import random
from typing import List

MAX_ITERATION = 10


class AIOpponent:
    def __init__(self, user_id: int, game_id: int) -> None:
        self.game_id = game_id
        self.user_id = user_id
        self._damaged_cells: List[int] = []
        self._damaged_ship_parts: List[int] = []
        self._unknown_cells = [[0 for _ in range(10)] for _ in range(10)]

    async def _getFirstUnknownCell(self):
        print("===========_getFirstUnknownCell===========")
        print(self._unknown_cells)
        for row in range(len(self._unknown_cells)):
            for column in range(len(self._unknown_cells[row])):
                if self._unknown_cells[column][row] == 0:
                    print([column, row])
                    print("===========_getFirstUnknownCell===========")
                    return [column, row]

    async def _getNextPotentionalCell(self) -> List[List[int]]:
        x_one_place = all(map(lambda x: x == self._damaged_ship_parts[0][1], 
                        [x[1] for x in self._damaged_ship_parts]))
        y_one_place = all(map(lambda y: y == self._damaged_ship_parts[0][0], 
                        [y[0] for y in self._damaged_ship_parts]))
        potentional_ship_part_cells = []

        # Это значит что координата одна
        if x_one_place and y_one_place:
            potentional_ship_part_cells = [
                [self._damaged_ship_parts[0][0] + 1, self._damaged_ship_parts[0][1]],
                [self._damaged_ship_parts[0][0] - 1, self._damaged_ship_parts[0][1]],
                [self._damaged_ship_parts[0][0], self._damaged_ship_parts[0][1] + 1],
                [self._damaged_ship_parts[0][0], self._damaged_ship_parts[0][1] - 1]
            ]
        else:
            for damaged_ship_cell in self._damaged_ship_parts:
                potentional_ship_part_cells.append([damaged_ship_cell[0] + x_one_place, damaged_ship_cell[1] + y_one_place])
                potentional_ship_part_cells.append([damaged_ship_cell[0] - x_one_place, damaged_ship_cell[1] - y_one_place])

        potentional_ship_part_cells = [cell for cell in potentional_ship_part_cells if 
                                0 <= cell[0] < 10 \
                                and 0 <= cell[1] < 10 \
                                and cell not in self._damaged_cells
        ]
            
        for potentional_ship_part_cell in potentional_ship_part_cells:
            return potentional_ship_part_cell

    async def makeNextTurn(self) -> List[int]:
        if self._damaged_ship_parts:
            x_pos, y_pos = await self._getNextPotentionalCell()
            print("============_getNextPotentionalCell============")
            print([x_pos, y_pos])
            print("============_getNextPotentionalCell============")
        else:
            i = 0
            while True:
                if i > MAX_ITERATION:
                    x_pos, y_pos = await self._getFirstUnknownCell()
                    break
                i += 1

                x_pos = random.randint(0, 9)
                y_pos = random.randint(0, 9)
                if [x_pos, y_pos] not in self._damaged_cells:
                    break
            print("============randomPosition============")
            print([x_pos, y_pos])
            print("============randomPosition============")
        
        return [x_pos, y_pos]
    
    async def updateDamagedCells(self, 
                                 new_damaged_cell: List[int], 
                                 new_damaged_ship_part: List[int],
                                 killed_ship_cells: List[List[int]]
    ) -> None:
        if new_damaged_cell:
            for cell_pos in new_damaged_cell:
                self._unknown_cells[cell_pos[0]][cell_pos[1]] = 1
                self._damaged_cells.append([cell_pos[0], cell_pos[1]])

        if new_damaged_ship_part:
            for cell_pos in new_damaged_ship_part:
                self._unknown_cells[cell_pos[0]][cell_pos[1]] = 1
                self._damaged_cells.append([cell_pos[0], cell_pos[1]])
                self._damaged_ship_parts.append([cell_pos[0], cell_pos[1]])

        if killed_ship_cells:
            self._damaged_ship_parts = []
            for killed_ship_cell in killed_ship_cells:
                if killed_ship_cell not in self._damaged_cells:
                    print("============updateKilledCells============")
                    print(killed_ship_cell)
                    print("============updateKilledCells============")
                    self._unknown_cells[killed_ship_cell[0]][killed_ship_cell[1]] = 1
                    self._damaged_cells.append(killed_ship_cell)

        print("============updateDamagedCells============")
        print(new_damaged_cell)
        print(new_damaged_ship_part)
        print(killed_ship_cells)
        print("============updateDamagedCells============")

        print("============currentCellsValues============")
        print(self._damaged_cells)
        print(self._damaged_ship_parts)
        print("============currentCellsValues============")
