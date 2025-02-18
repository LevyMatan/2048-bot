from enum import Enum
from typing import Dict


class Action(Enum):
    LEFT = 0
    RIGHT = 1
    UP = 2
    DOWN = 3


class Board:
    __is_lookup_tables_initialized: bool = False
    __left_moves: list[int] = [0] * (2**16)
    __right_moves: list[int] = [0] * (2**16)
    __empty_cells: Dict[int, list[tuple[int, int]]] = {}

    def __init__(self, state: int = None):
        if state is not None:
            Board.__verify_state(state=state)
            self.__state = state
        else:
            self.__state = 0
        if not Board.__is_lookup_tables_initialized:
            Board.__init_lookup_tables()
            Board.__is_lookup_tables_initialized = True

    @staticmethod
    def get_empty_cells(state: int) -> Dict[int, list[tuple[int, int]]]:
        Board.__verify_state(state)
        if state in Board.__empty_cells:
            return Board.__empty_cells[state]
        else:
            empty_tiles = []
            for idx, value in enumerate(Board.get_unpacked_state(state)):
                if value == 0:
                    empty_tiles.append((idx // 4, idx % 4))
            Board.__empty_cells[state] = empty_tiles
        return Board.__empty_cells

    @staticmethod
    def is_lookup_tables_initialized() -> bool:
        return Board.__is_lookup_tables_initialized

    @staticmethod
    def __move_left_verifier(row: list[int]) -> None:
        if len(row) != 4:
            raise ValueError("Invalid row length")
        if not all(0 <= x <= 15 for x in row):
            raise ValueError(f"Invalid row values: row = {row}")
        if not isinstance(row, list) or not all(isinstance(x, int) for x in row):
            raise TypeError("Invalid row type")

    @staticmethod
    def _move_left(row: list[int]) -> list[int]:
        # Validate input
        Board.__move_left_verifier(row)

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
                # Check for 15+15 edge case
                if non_zero[i] == 15:
                    result[j] = non_zero[i]
                    j += 1
                    result[j] = non_zero[i + 1]
                    skip = True
                else:
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
    def __init_lookup_tables():
        # Guard against multiple initializations
        if Board.__is_lookup_tables_initialized:
            return

        for i in range(2**16):
            row = [
                (i >> 12) & 0xF,
                (i >> 8) & 0xF,
                (i >> 4) & 0xF,
                i & 0xF
            ]
            new_row_left = Board._move_left(row)
            new_value_left = (new_row_left[0] << 12) | (new_row_left[1] << 8) | (new_row_left[2] << 4) | new_row_left[3]
            Board.__left_moves[i] = new_value_left

            new_row_right = Board._move_right(row)
            new_value_right = (new_row_right[0] << 12) | (new_row_right[1] << 8) | (new_row_right[2] << 4) | new_row_right[3]
            Board.__right_moves[i] = new_value_right

    @staticmethod
    def __verify_row_left(row: int) -> None:
        if not isinstance(row, int):
            raise TypeError(f"Invalid row type: {row}")
        if row < 0:
            raise IndexError(f"Invalid row: {row}")

    @staticmethod
    def _row_left(row: int) -> int:
        # Verify row.
        Board.__verify_row_left(row)
        return Board.__left_moves[row]

    @staticmethod
    def __verify_row_right(row: int) -> None:
        if not isinstance(row, int):
            raise TypeError(f"Invalid row type: {row}")
        if row < 0 or row >= (2**16):
            raise IndexError(f"Invalid row: {row}")

    @staticmethod
    def _row_right(row: int) -> int:
        Board.__verify_row_right(row)
        if not Board.__is_lookup_tables_initialized:
            Board.__init_lookup_tables()
        return Board.__right_moves[row]

    @staticmethod
    def __verify_state(state: int) -> bool:
        if not isinstance(state, int):
            raise TypeError(f"Invalid state type: {state}")
        if state < 0:
            raise ValueError(f"Invalid state: {state}")
        if state >= (2**64):
            raise OverflowError(f"Invalid state: {state}")
        return True

    @staticmethod
    def simulate_moves(state: int) -> list[int]:
        # Verify input
        Board.__verify_state(state)

        # Horizontal moves: LEFT and RIGHT.
        new_state_left = new_state_right = 0
        for row_index in range(4):
            row = (state >> (16 * row_index)) & 0xFFFF
            new_row_left = Board.__left_moves[row]
            new_row_right = Board.__right_moves[row]
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
            new_col_value_up = Board.__right_moves[col_value]
            new_col_value_down = Board.__left_moves[col_value]
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
        # Verify input
        Board.__verify_state(state)

        if state in Board.__empty_cells:
            return Board.__empty_cells[state]
        else:
            empty_tiles = []
            for idx, value in enumerate(Board.get_unpacked_state(state)):
                if value == 0:
                    empty_tiles.append((idx // 4, idx % 4))
            Board.__empty_cells[state] = empty_tiles
        return empty_tiles

    @staticmethod
    def __verify_row_col(row: int, col: int):
        if row < 0 or row > 3 or col < 0 or col > 3:
            raise ValueError("Row and column must be between 0 and 3.")

    @staticmethod
    def __verify_value(value: int):
        if value < 0 or value > 15:
            raise ValueError("Value must be between 0 and 15.")

    @staticmethod
    def set_tile(state: int, row: int, col: int, value: int):
        Board.__verify_state(state)
        Board.__verify_row_col(row, col)
        Board.__verify_value(value)

        # Convert row/col to bit position from left to right
        i = (3 - row) * 4 + (3 - col)
        if ((state >> (i * 4)) & 0xF) != 0:
            raise ValueError("Tile at given row and column is not empty.")
        state |= value << (i * 4)
        return state

    # set_state and get_state stay instance methods.
    def set_state(self, state: int):
        # Verify state.
        Board.__verify_state(state)
        self.__state = state

    def get_state(self, unpack: bool = False) -> int | list[int]:
        # Verify unpack is of type bool
        if not isinstance(unpack, bool):
            raise TypeError("unpack must be of type bool.")
        if unpack:
            return Board.get_unpacked_state(self.__state)
        return self.__state

    @staticmethod
    def get_unpacked_state(state: int) -> list[int]:
        Board.__verify_state(state)
        return [(state >> (i * 4)) & 0xF for i in range(15, -1, -1)]

    @staticmethod
    def reset():
        Board.__empty_cells = {}

    @staticmethod
    def pretty_print(state: int):
        Board.__verify_state(state)
        print("-" * 11)
        for i in range(4):
            row = (state >> ((3 - i) * 16)) & 0xFFFF
            print("{:X} {:X} {:X} {:X}".format(
                row >> 12, (row >> 8) & 0xF, (row >> 4) & 0xF, row & 0xF))

    @staticmethod
    def disable_verifiers():
        Board.original_verifiers = {
            "__verify_state": Board.__verify_state,
            "__verify_row_col": Board.__verify_row_col,
            "__verify_value": Board.__verify_value,
            "__move_left_verifier": Board.__move_left_verifier,
            "__verify_row_left": Board.__verify_row_left,
            "__verify_row_right": Board.__verify_row_right
        }
        Board.__verify_state = lambda state: None
        Board.__verify_row_col = lambda row, col: None
        Board.__verify_value = lambda value: None
        Board.__move_left_verifier = lambda row: None
        Board.__verify_row_left = lambda row: None
        Board.__verify_row_right = lambda row: None

    @staticmethod
    def enable_verifiers():
        Board.__verify_state = Board.original_verifiers["__verify_state"]
        Board.__verify_row_col = Board.original_verifiers["__verify_row_col"]
        Board.__verify_value = Board.original_verifiers["__verify_value"]
        Board.__move_left_verifier = Board.original_verifiers["__move_left_verifier"]
        Board.__verify_row_left = Board.original_verifiers["__verify_row_left"]
        Board.__verify_row_right = Board.original_verifiers["__verify_row_right"]
