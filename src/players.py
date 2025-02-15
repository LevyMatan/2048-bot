from abc import ABC, abstractmethod
from .board import Action, Board
import random

class Player(ABC):
    def __init__(self):
        self.name = ""

    @abstractmethod
    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        """Return a chosen action and the resulting state given a list of valid actions."""
        pass

class RandomPlayer(Player):
    def __init__(self):
        self.name = "Random"

    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        return random.choice(valid_actions)

class MaxEmptyCellsPlayer(Player):
    def __init__(self):
        self.name = "MaxEmptyCells"

    def evaluate_state(self, state: int) -> int:
        return len(Board.get_empty_tiles(state))

    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        return max(valid_actions, key=lambda x: self.evaluate_state(x[1]))

class MinMaxPlayer(Player):
    def __init__(self):
        self.name = "MinMax"

    def evaluate_state(self, state: int) -> int:
        return sum([2 ** tile for tile in Board.get_unpacked_state(state) if tile > 0])

    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        return max(valid_actions, key=lambda x: self.evaluate_state(x[1]))

class HeuristicPlayer(Player):
    def __init__(self):
        self.name = "Heuristic"

    def evaluate_state(self, state: int) -> float:
        """
        Evaluate a board state using several factors:
          - Empty cells: more is better.
          - Monotonicity: boards that are more ordered (either increasing or decreasing)
            tend to be easier to merge.
          - Smoothness: penalize large differences between adjacent tiles.
          - Max tile: reward boards with a high maximum tile.
        Note: The weights used below are rough estimates and may need tuning.
        """
        board = Board.get_unpacked_state(state)  # board is a list of 16 numbers (0 means empty, >0 means 2**tile)

        # Factor 1: Count empty cells.
        empty_cells = sum(1 for cell in board if cell == 0)

        # Factor 2: Monotonicity.
        # Evaluate monotonicity along rows and columns using differences in tile exponents.
        row_monotonicity = 0
        for r in range(4):
            row = board[r * 4:(r + 1) * 4]
            # Calculate differences between adjacent cells.
            row_monotonicity += abs(row[0] - row[1]) + abs(row[1] - row[2]) + abs(row[2] - row[3])
        col_monotonicity = 0
        for c in range(4):
            col = [board[r * 4 + c] for r in range(4)]
            col_monotonicity += abs(col[0] - col[1]) + abs(col[1] - col[2]) + abs(col[2] - col[3])
        monotonicity = -(row_monotonicity + col_monotonicity)

        # Factor 3: Smoothness.
        # Penalize adjacent tiles that have a high discrepancy (here we use actual tile values, i.e. 2**exponent).
        smoothness = 0
        for r in range(4):
            for c in range(4):
                index = r * 4 + c
                if board[index] != 0:
                    value = 2 ** board[index]
                    # Check right neighbor.
                    if c < 3 and board[r * 4 + c + 1] != 0:
                        neighbor_value = 2 ** board[r * 4 + c + 1]
                        smoothness -= abs(value - neighbor_value)
                    # Check bottom neighbor.
                    if r < 3 and board[(r + 1) * 4 + c] != 0:
                        neighbor_value = 2 ** board[(r + 1) * 4 + c]
                        smoothness -= abs(value - neighbor_value)

        # Factor 4: Maximum tile.
        max_tile = max(board)

        # Weights for each factor (needs tuning)
        empty_weight = 270.0
        monotonicity_weight = 47.0
        smoothness_weight = 15.0
        max_tile_weight = 100.0

        score = (empty_cells * empty_weight) + \
                (monotonicity * monotonicity_weight) + \
                (smoothness * smoothness_weight) + \
                (max_tile * max_tile_weight)
        return score

    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        # Choose the action that maximizes the evaluation function.
        return max(valid_actions, key=lambda action_state: self.evaluate_state(action_state[1]))

class MonteCarloPlayer(Player):
    def __init__(self, simulations: int = 50):
        self.name = "MonteCarlo"
        self.simulations = simulations

    def simulate_random_game(self, state: int) -> int:
        # Simulate a game randomly from the given state until no further moves.
        current_state = state
        while True:
            valid_moves = Board.get_valid_move_actions(current_state)
            if not valid_moves:
                break
            # Randomly pick a move.
            _, next_state = random.choice(valid_moves)
            current_state = next_state
            # Add a random tile.
            current_state = Board.set_tile(current_state, *random.choice(Board.get_empty_tiles(current_state)), 1 if random.random() < 0.9 else 2)
        # Return score as in your get_score function: sum of 2**cell values.
        return sum(2 ** tile for tile in Board(current_state).get_state(unpack=True) if tile > 0)

    def evaluate_state(self, state: int) -> float:
        # Run a number of simulations and average the score.
        total_score = 0
        for _ in range(self.simulations):
            total_score += self.simulate_random_game(state)
        return total_score / self.simulations

    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        # Choose the action with best average outcome.
        return max(valid_actions, key=lambda action_state: self.evaluate_state(action_state[1]))

class HumanPlayer(Player):
    def __init__(self):
        self.name = "Human"

    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        while True:
            action = None
            action_str = input("Enter action (a/A: left, d/D: right, w/W: up, s/S: down): ")
            if action_str.lower() == 'a':
                action = Action.LEFT
            elif action_str.lower() == 'd':
                action = Action.RIGHT
            elif action_str.lower() == 'w':
                action = Action.UP
            elif action_str.lower() == 's':
                action = Action.DOWN
            try:
                for act, state in valid_actions:
                    if act == action:
                        return act, state
                else:
                    print("Invalid action.")
                    continue
            except ValueError:
                print("Invalid input.")

class ExpectimaxPlayer(Player):
    def __init__(self, depth: int = 8):
        self.name = "Expectimax"
        self.depth = depth
        self.empty_weight = 270.0
        self.monotonicity_weight = 470.0
        self.smoothness_weight = 15.0
        self.max_tile_weight = 100.0
        print(f"Using {self.name} with depth = {self.depth}")
        print(f"Weights: empty = {self.empty_weight}, "
              f"monotonicity = {self.monotonicity_weight}, "
              f"smoothness = {self.smoothness_weight}, "
              f"max_tile = {self.max_tile_weight}")

    def evaluate_state(self, state: int) -> float:
        """
        Use a heuristic similar to that used by HeuristicPlayer.
        Factors include empty cells, monotonicity, smoothness, and maximum tile.
        Adjust weights as needed.
        """
        board = Board.get_unpacked_state(state)
        empty_cells = sum(1 for cell in board if cell == 0)

        row_monotonicity = 0
        for r in range(4):
            row = board[r * 4:(r + 1) * 4]
            row_monotonicity += abs(row[0] - row[1]) + abs(row[1] - row[2]) + abs(row[2] - row[3])
        col_monotonicity = 0
        for c in range(4):
            col = [board[r * 4 + c] for r in range(4)]
            col_monotonicity += abs(col[0] - col[1]) + abs(col[1] - col[2]) + abs(col[2] - col[3])
        monotonicity = -(row_monotonicity + col_monotonicity)

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

        max_tile = max(board)

        return (empty_cells * self.empty_weight +
                monotonicity * self.monotonicity_weight +
                smoothness * self.smoothness_weight +
                max_tile * self.max_tile_weight)

    def expectimax(self, state: int, depth: int, is_player_turn: bool) -> float:
        valid_moves = Board.get_valid_move_actions(state)
        # Terminal conditions: depth cutoff or no valid moves.
        if depth == 0 or not valid_moves:
            return self.evaluate_state(state)
        if is_player_turn:
            best = float('-inf')
            for _, next_state in valid_moves:
                best = max(best, self.expectimax(next_state, depth - 1, False))
            return best
        else:
            # Chance node: average over all possible placements.
            empty_tiles = Board.get_empty_tiles(state)
            if not empty_tiles:
                return self.evaluate_state(state)
            total = 0.0
            # Each empty tile has an equal chance, and for each, tile 1 comes with 0.9 and tile 2 with 0.1.
            prob_per_tile = 1 / len(empty_tiles)
            for (row, col) in empty_tiles:
                for tile_value, tile_prob in [(1, 0.9), (2, 0.1)]:
                    # Create a new state with the tile added.
                    # We assume Board.set_tile returns a new state without modifying the original.
                    new_state = Board.set_tile(state, row, col, tile_value)
                    total += prob_per_tile * tile_prob * self.expectimax(new_state, depth - 1, True)
            return total

    def choose_action(self, valid_actions: list[tuple[Action, int]]) -> tuple[Action, int]:
        best_move = None
        best_value = float('-inf')
        for action, next_state in valid_actions:
            value = self.expectimax(next_state, self.depth, False)
            if value > best_value:
                best_value = value
                best_move = (action, next_state)
        return best_move
# You can add additional player types by inheriting from Player and implementing choose_action.