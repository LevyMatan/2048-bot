from abc import ABC, abstractmethod
from .board import Board
import os
import time

# Define color codes for each tile value (foreground and background)
TILE_COLORS = {
    1: ('\033[97m', '\033[48;5;223m'),  # 2: white text on light peach
    2: ('\033[97m', '\033[48;5;216m'),  # 4: white text on salmon
    3: ('\033[97m', '\033[48;5;209m'),  # 8: white text on coral
    4: ('\033[97m', '\033[48;5;203m'),  # 16: white text on light red
    5: ('\033[97m', '\033[48;5;197m'),  # 32: white text on bright red
    6: ('\033[97m', '\033[48;5;164m'),  # 64: white text on magenta
    7: ('\033[97m', '\033[48;5;127m'),  # 128: white text on purple
    8: ('\033[97m', '\033[48;5;214m'),  # 256: white text on orange
    9: ('\033[97m', '\033[48;5;220m'),  # 512: white text on gold
    10: ('\033[97m', '\033[48;5;226m'), # 1024: white text on yellow
    11: ('\033[30m', '\033[48;5;228m'), # 2048: black text on light yellow
    12: ('\033[30m', '\033[48;5;231m'), # 4096: black text on bright white
}
RESET_COLOR = '\033[0m'
CELL_WIDTH = 6

class Interface2048(ABC):
    def __init__(self):
        self.name = ""

    def display_initial_board(self, state: int, score: int = 0):
        pass

    @abstractmethod
    def update(self, state: int, move_count: int, score: int) -> None:
        pass

class GUI2048(Interface2048):
    def __init__(self):
        self.name = "GUI"

    def display_initial_board(self, state: int, score: int = 0):
        pass

    def update(self, state: int, move_count: int, score: int) -> None:
        pass

class CLI2048(Interface2048):
    def __init__(self):
        self.name = "CLI"
        self.first_update = True

    @staticmethod
    def pretty_print(board: int, score: int, move_count: int, clear_screen: bool = True):
        if clear_screen:
            os.system('cls' if os.name == 'nt' else 'clear')
        
        print(f"Score: {score}")
        if move_count:
            print(f"Move count: {move_count}")

        cells = [tile for tile in Board.get_unpacked_state(board)]
        
        # Convert each cell: if non-zero, display 2**cell with color, else empty string
        display_cells = []
        for cell in cells:
            if cell > 0:
                fg_color, bg_color = TILE_COLORS.get(cell, TILE_COLORS[12])
                value = str(2**cell)
                # Calculate padding to center the number in CELL_WIDTH
                left_padding = ' ' * ((CELL_WIDTH - len(value)) // 2)
                right_padding = ' ' * (CELL_WIDTH - len(value) - len(left_padding))
                # Apply colors to the entire cell including padding
                display_cells.append(f"{fg_color}{bg_color}{left_padding}{value}{right_padding}{RESET_COLOR}")
            else:
                # Empty cell with consistent width
                display_cells.append(' ' * CELL_WIDTH)

        # Define borders with consistent width
        horizontal_line = '+' + ('-' * CELL_WIDTH + '+') * 4

        print(horizontal_line)
        for row in range(4):
            row_cells = display_cells[row * 4 : (row + 1) * 4]
            row_str = '|' + '|'.join(row_cells) + '|'
            print(row_str)
            print(horizontal_line)

    def display_initial_board(self, state: int, score: int = 0):
        """
        Display the initial board state right after initialization.
        This should be called once at the beginning of the game.
        """
        CLI2048.pretty_print(state, score, 0)  # Move count is 0 at initialization
        print("\nUse arrow keys or WASD to move. Press 'q' to quit.")
        print("Waiting for your move...", flush=True)
        self.first_update = False  # Set this to False so instructions aren't shown again

    def update(self, state: int, move_count: int, score: int) -> None:
        CLI2048.pretty_print(state, score, move_count)
        
        # Display instructions on first update and ensure output is flushed
        if self.first_update:
            print("\nUse arrow keys or WASD to move. Press 'q' to quit.")
            print("Waiting for your move...", flush=True)
            self.first_update = False

class GYM2048(Interface2048):
    def __init__(self):
        self.name = "GYM"
        self.total_games = 0
        self.current_game = 0
        self.best_score = 0
        self.start_time = None

    def display_initial_board(self, state: int, score: int = 0):
        """Called at the start of each game"""
        # Initialize start time on first game
        if self.current_game == 0:
            self.start_time = time.time()
        
        # Increment game counter
        self.current_game += 1
        
        # Show progress every 1%
        if self.current_game % max(1, self.total_games // 100) == 0:
            progress = (self.current_game / self.total_games) * 100
            elapsed_time = time.time() - self.start_time
            # Use \r to overwrite the line and \033[K to clear to the end of line
            print(f"\rProgress: {progress:5.1f}% ({self.current_game}/{self.total_games} games) - "
                  f"Best score: {self.best_score} - "
                  f"Time: {elapsed_time:.1f}s", end="\033[K", flush=True)

    def set_total_games(self, total_games: int):
        """Set the total number of games to be played."""
        self.total_games = total_games
        print(f"Starting simulation of {total_games} games...")

    def update(self, state: int, move_count: int, score: int) -> None:
        # Update best score if current score is higher
        if score > self.best_score:
            self.best_score = score
