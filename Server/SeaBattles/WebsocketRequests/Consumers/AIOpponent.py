import random
from typing import List


class AIOpponent:
    def __init__(self, user_id: int, game_id: int) -> None:
        self.game_id = game_id
        self.user_id = user_id
        self._damaged_cells: List[int] = []
        self._damaged_ship_parts: List[int] = []

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
            x_pos, y_pos = self._getNextPotentionalCell(self._damaged_ship_parts, self._damaged_cells)
        else:
            x_pos = random.randint(0, 9)
            y_pos = random.randint(0, 9)
        return [x_pos, y_pos]
    
    async def updateDamagedCells(self, 
                                 new_damaged_cell: List[int], 
                                 new_damaged_ship_part: List[int],
                                 killed_ship_cells: List[List[int]]
    ) -> None:
        if new_damaged_cell:
            self._damaged_cells.append(new_damaged_cell)

        if new_damaged_ship_part:
            self._damaged_ship_parts.append(new_damaged_ship_part)

        if killed_ship_cells:
            self._damaged_ship_parts = []
            for killed_ship_cell in killed_ship_cells:
                if killed_ship_cell not in self._damaged_cells:
                    self._damaged_cells.append(killed_ship_cell)
