from abc import ABC, abstractmethod
from board import Action, Board
import random

class Interface2048(ABC):
    def __init__(self):
        self.name = ""

    @abstractmethod
    def update(self, state: int) -> None:
        pass

class GUI2048(Interface2048):
    def __init__(self):
        self.name = "GUI"

    def update(self, state: int) -> None:
        pass

class CLI2048(Interface2048):
    def __init__(self):
        self.name = "GUI"

    @staticmethod
    def pretty_print(board: int, score: int, move_count: int):
        """
        Pretty-print the board with borders.
        Each tile is displayed as 2**cell if cell > 0, otherwise left blank.
        """
        board = board

        # Print Score
        if score:
            print(f"Score: {score}")
        # Print move count
        if move_count:
            print(f"Move count: {move_count}")

        # Unpack the board state to a list of 16 cells in row-major order.
        cells = [tile for tile in Board(board).get_unpacked_state(board)]
        # Convert each cell: if non-zero, display 2**cell, else empty string.
        display_cells = [str(2**cell) if cell > 0 else '' for cell in cells]

        # Define column width (adjust as needed)
        cell_width = 6
        horizontal_line = '+' + '+'.join(['-' * cell_width] * 4) + '+'

        print(horizontal_line)
        for row in range(4):
            row_cells = display_cells[row * 4 : (row + 1) * 4]
            # Center each cell value in the column.
            row_str = '|' + '|'.join(cell.center(cell_width) for cell in row_cells) + '|'
            print(row_str)
            print(horizontal_line)

    def update(self, state: int, move_count: int) -> None:
        board = Board(state)
        score = sum([2 ** tile for tile in board.get_state(unpack=True) if tile > 0])
        CLI2048.pretty_print(board, score, move_count)
        pass

class GYM2048(Interface2048):
    def __init__(self):
        self.name = "GUI"

    def update(self, state: int) -> None:
        pass
