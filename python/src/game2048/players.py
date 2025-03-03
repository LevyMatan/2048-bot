from abc import ABC, abstractmethod
from .board import Action, Board
import random
import readchar

class Player(ABC):
    def __init__(self):
        self.name = ""

    @abstractmethod
    def choose_action(self, valid_actions: list[tuple[Action, int, int]]) -> tuple[Action, int, int]:
        """Return a chosen action, the resulting state, and the score for this move."""
        pass

class RandomPlayer(Player):
    def __init__(self):
        self.name = "Random"

    def choose_action(self, valid_actions: list[tuple[Action, int, int]]) -> tuple[Action, int, int]:
        return random.choice(valid_actions)

class MaxEmptyCellsPlayer(Player):
    def __init__(self):
        self.name = "MaxEmptyCells"

    def evaluate_state(self, state: int) -> int:
        return len(Board.get_empty_tiles(state))

    def choose_action(self, valid_actions: list[tuple[Action, int, int]]) -> tuple[Action, int, int]:
        return max(valid_actions, key=lambda x: self.evaluate_state(x[1]))

class MinMaxPlayer(Player):
    def __init__(self):
        self.name = "MinMax"

    def evaluate_state(self, state: int) -> int:
        return sum([2 ** tile for tile in Board.get_unpacked_state(state) if tile > 0])

    def choose_action(self, valid_actions: list[tuple[Action, int, int]]) -> tuple[Action, int, int]:
        return max(valid_actions, key=lambda x: self.evaluate_state(x[1]))

class BaseHeuristicPlayer(Player):
    """Base class for players that use heuristic evaluation."""
    def __init__(self, name="BaseHeuristic", empty_weight=270.0, monotonicity_weight=470.0, 
                 smoothness_weight=15.0, max_tile_weight=100.0):
        self.name = name
        self.empty_weight = empty_weight
        self.monotonicity_weight = monotonicity_weight
        self.smoothness_weight = smoothness_weight
        self.max_tile_weight = max_tile_weight

    def evaluate_state(self, state: int) -> float:
        """
        Evaluate a board state using several factors:
          - Empty cells: more is better.
          - Monotonicity: boards that are more ordered (either increasing or decreasing)
            tend to be easier to merge.
          - Smoothness: penalize large differences between adjacent tiles.
          - Max tile: reward boards with a high maximum tile.
        """
        board = Board.get_unpacked_state(state)
        
        # Factor 1: Count empty cells.
        empty_cells = sum(1 for cell in board if cell == 0)

        # Factor 2: Monotonicity
        row_monotonicity = 0
        for r in range(4):
            row = board[r * 4:(r + 1) * 4]
            row_monotonicity += abs(row[0] - row[1]) + abs(row[1] - row[2]) + abs(row[2] - row[3])
        col_monotonicity = 0
        for c in range(4):
            col = [board[r * 4 + c] for r in range(4)]
            col_monotonicity += abs(col[0] - col[1]) + abs(col[1] - col[2]) + abs(col[2] - col[3])
        monotonicity = -(row_monotonicity + col_monotonicity)

        # Factor 3: Smoothness
        smoothness = 0
        for r in range(4):
            for c in range(4):
                index = r * 4 + c
                if board[index] != 0:
                    value = 2 ** board[index]
                    if c < 3 and board[r * 4 + c + 1] != 0:
                        neighbor_value = 2 ** board[r * 4 + c + 1]
                        smoothness -= abs(value - neighbor_value)
                    if r < 3 and board[(r + 1) * 4 + c] != 0:
                        neighbor_value = 2 ** board[(r + 1) * 4 + c]
                        smoothness -= abs(value - neighbor_value)

        # Factor 4: Maximum tile
        max_tile = max(board)

        return (empty_cells * self.empty_weight +
                monotonicity * self.monotonicity_weight +
                smoothness * self.smoothness_weight +
                max_tile * self.max_tile_weight)

class HeuristicPlayer(BaseHeuristicPlayer):
    def __init__(self):
        super().__init__(name="Heuristic")

    def choose_action(self, valid_actions: list[tuple[Action, int, int]]) -> tuple[Action, int, int]:
        return max(valid_actions, key=lambda action_state_score: self.evaluate_state(action_state_score[1]))

class HumanPlayer(Player):
    def __init__(self):
        self.name = "Human"
        self._first_move = True
        
    def choose_action(self, valid_actions: list[tuple[Action, int, int]]) -> tuple[Action, int, int]:
        # Extract just the actions for validation
        valid_action_types = [action for action, _, _ in valid_actions]
        
        while True:
            try:
                # Wait for a single keypress without requiring Enter
                key = readchar.readkey()
                
                action = None
                # Map both arrow keys and WASD to actions
                if key in [readchar.key.LEFT, 'a', 'A']:
                    action = Action.LEFT
                elif key in [readchar.key.RIGHT, 'd', 'D']:
                    action = Action.RIGHT
                elif key in [readchar.key.UP, 'w', 'W']:
                    action = Action.UP
                elif key in [readchar.key.DOWN, 's', 'S']:
                    action = Action.DOWN
                elif key in ['q', 'Q']:  # Allow graceful exit with q
                    print("\nExiting game. Thanks for playing!")
                    import sys
                    sys.exit(0)  # Exit cleanly with status code 0
                
                # Check if the action is valid
                if action in valid_action_types:
                    for act, state, score in valid_actions:
                        if act == action:
                            return act, state, score
                elif action is not None:
                    # Only show message for invalid moves (not for unrecognized keys)
                    print("Invalid move!")
                    
            except KeyboardInterrupt:
                # Handle Ctrl+C gracefully
                print("\nExiting game. Thanks for playing!")
                import sys
                sys.exit(0)

# You can add additional player types by inheriting from Player and implementing choose_action.