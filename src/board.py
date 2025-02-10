from enum import Enum

class Action(Enum):
    LEFT = 0
    RIGHT = 1
    UP = 2
    DOWN = 3

class Board:
    is_lookup_tables_initialized: bool = False
    left_moves: list[int] = [0] * (2**16)
    right_moves: list[int] = [0] * (2**16)

    def __init__(self, state: int = None):
        if state is not None:
            self.state = state
        else:
            self.state = 0
        if not Board.is_lookup_tables_initialized:
            Board._init_lookup_tables()
            Board.is_lookup_tables_initialized = True

    @staticmethod
    def _move_left(row: list[int]) -> list[int]:
        non_zero = [x for x in row if x != 0]
        result = [0, 0, 0, 0]
        skip = False
        i = j = 0
        while i < len(non_zero):
            if skip:
                skip = False
                i += 1
                continue
            if i + 1 < len(non_zero) and non_zero[i] == non_zero[i + 1]:
                result[j] = non_zero[i] + 1
                skip = True
            else:
                result[j] = non_zero[i]
            i += 1
            j += 1
        return result

    @staticmethod
    def _move_right(row: list[int]) -> list[int]:
        reversed_row = list(reversed(row))
        moved = Board._move_left(reversed_row)
        return list(reversed(moved))

    @staticmethod
    def _init_lookup_tables():
        for i in range(2**16):
            row = [
                (i >> 12) & 0xF,
                (i >> 8) & 0xF,
                (i >> 4) & 0xF,
                i & 0xF
            ]
            new_row_left = Board._move_left(row)
            new_value_left = (new_row_left[0] << 12) | (new_row_left[1] << 8) | (new_row_left[2] << 4) | new_row_left[3]
            Board.left_moves[i] = new_value_left

            new_row_right = Board._move_right(row)
            new_value_right = (new_row_right[0] << 12) | (new_row_right[1] << 8) | (new_row_right[2] << 4) | new_row_right[3]
            Board.right_moves[i] = new_value_right

    @staticmethod
    def _row_left(row: int) -> int:
        return Board.left_moves[row]

    @staticmethod
    def _row_right(row: int) -> int:
        return Board.right_moves[row]

    @staticmethod
    def simulate_move(state: int, action: Action) -> int:
        new_state = 0
        if action in (Action.LEFT, Action.RIGHT):
            for row_index in range(4):
                row = (state >> (16 * row_index)) & 0xFFFF
                if action == Action.LEFT:
                    new_row = Board._row_left(row)
                else:
                    new_row = Board._row_right(row)
                new_state |= new_row << (16 * row_index)
        elif action in (Action.UP, Action.DOWN):
            rows = [(state >> (16 * i)) & 0xFFFF for i in range(4)]
            new_rows = [0, 0, 0, 0]
            for col in range(4):
                col_value = 0
                for row_index in range(4):
                    tile = (rows[row_index] >> (4 * (3 - col))) & 0xF
                    col_value |= tile << (4 * (3 - row_index))
                if action == Action.UP:
                    new_col_value = Board.left_moves[col_value]
                else:
                    new_col_value = Board.right_moves[col_value]
                for row_index in range(4):
                    new_tile = (new_col_value >> (4 * (3 - row_index))) & 0xF
                    new_rows[row_index] |= new_tile << (4 * (3 - col))
            for row_index in range(4):
                new_state |= new_rows[row_index] << (16 * row_index)
        else:
            raise ValueError(f"Unsupported action: {action}")
        return new_state

    @staticmethod
    def simulate_moves(state: int) -> list[int]:
        # Horizontal moves: LEFT and RIGHT.
        new_state_left = new_state_right = 0
        for row_index in range(4):
            row = (state >> (16 * row_index)) & 0xFFFF
            new_row_left = Board.left_moves[row]
            new_row_right = Board.right_moves[row]
            new_state_left |= new_row_left << (16 * row_index)
            new_state_right |= new_row_right << (16 * row_index)

        # Vertical moves: reuse the row extraction.
        rows = [(state >> (16 * i)) & 0xFFFF for i in range(4)]
        new_rows_up = [0, 0, 0, 0]
        new_rows_down = [0, 0, 0, 0]
        for col in range(4):
            col_value = 0
            for row_index in range(4):
                tile = (rows[row_index] >> (4 * (3 - col))) & 0xF
                col_value |= tile << (4 * (3 - row_index))
            new_col_value_up = Board.left_moves[col_value]
            new_col_value_down = Board.right_moves[col_value]
            for row_index in range(4):
                new_tile_up = (new_col_value_up >> (4 * (3 - row_index))) & 0xF
                new_tile_down = (new_col_value_down >> (4 * (3 - row_index))) & 0xF
                new_rows_up[row_index] |= new_tile_up << (4 * (3 - col))
                new_rows_down[row_index] |= new_tile_down << (4 * (3 - col))
        new_state_up = new_state_down = 0
        for row_index in range(4):
            new_state_up |= new_rows_up[row_index] << (16 * row_index)
            new_state_down |= new_rows_down[row_index] << (16 * row_index)

        return [new_state_left, new_state_right, new_state_up, new_state_down]

    @staticmethod
    def get_valid_move_actions(state: int) -> list[tuple[Action, int]]:
        valid_actions = []
        next_states = Board.simulate_moves(state)
        for action_value, next_state in enumerate(next_states):
            if next_state != state:
                valid_actions.append((Action(action_value), next_state))
        return valid_actions

    @staticmethod
    def get_empty_tiles(state: int) -> list[tuple[int, int]]:
        empty_tiles = []
        for i in range(16):
            if (state >> (i * 4)) & 0xF == 0:
                empty_tiles.append((i // 4, i % 4))
        return empty_tiles

    @staticmethod
    def set_tile(state: int, row: int, col: int, value: int):
        if value < 0 or value > 15:
            raise ValueError("Value must be between 0 and 15.")
        if row < 0 or row > 3 or col < 0 or col > 3:
            raise ValueError("Row and column must be between 0 and 3.")
        i = row * 4 + col
        if ((state >> (i * 4)) & 0xF) != 0:
            raise ValueError("Tile at given row and column is not empty.")
        state |= value << (i * 4)
        return state

    # set_state and get_state stay instance methods.
    def set_state(self, state: int):
        self.state = state

    def get_state(self, unpack: bool = False) -> int | list[int]:
        if unpack:
            return [(self.state >> (i * 4)) & 0xF for i in range(16)]
        return self.state

    @staticmethod
    def get_unpacked_state(state: int) -> list[int]:
        return [(state >> (i * 4)) & 0xF for i in range(16)]

    @staticmethod
    def pretty_print(state: int):
        print("-" * 11)
        for i in range(4):
            row = (state >> (i * 16)) & 0xFFFF
            print("{:02X} {:02X} {:02X} {:02X}".format(
                row >> 12, (row >> 8) & 0xF, (row >> 4) & 0xF, row & 0xF))