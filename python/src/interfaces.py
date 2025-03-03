from abc import ABC, abstractmethod
from .board import Board
import os
import time

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
        """
        Pretty-print the board with borders.
        Each tile is displayed as 2**cell if cell > 0, otherwise left blank.
        """
        # Clear the screen for better visibility (optional)
        if clear_screen:
            os.system('cls' if os.name == 'nt' else 'clear')
        
        # Print Score
        print(f"Score: {score}")
        # Print move count
        if move_count:
            print(f"Move count: {move_count}")

        # Unpack the board state to a list of 16 cells in row-major order.
        cells = [tile for tile in Board.get_unpacked_state(board)]
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
