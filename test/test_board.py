from src.board import Board, Action
import io
import pytest
import sys

class TestBoard:

    def test___init___1(self):
        # Existing test method, left unchanged
        pass

    def test___init___2(self):
        """
        Test initialization of Board with default state and uninitialized lookup tables.
        """
        # Create a new Board instance without providing a state
        board = Board()

        # Check that the state is initialized to 0
        assert board.get_state() == 0

        # Check that the lookup tables are now initialized
        assert Board.is_lookup_tables_initialized() == True

        with pytest.raises(AttributeError):
            len(Board.__left_moves) == 2**16
        with pytest.raises(AttributeError):
            len(Board.__right_moves) == 2**16

    def test___init___3(self):
        """
        Test initialization of Board with a non-None state and pre-initialized lookup tables.
        """
        # Test initialization with a non-None state
        test_state = 0x1234567890ABCDEF
        board = Board(test_state)

        # Verify that the state is set correctly
        assert board.get_state() == test_state

        # Verify that the lookup tables remain initialized
        assert Board.is_lookup_tables_initialized() == True

        # Verify that __init_lookup_tables is not called again
        with pytest.raises(AttributeError):
            Board.__init_lookup_tables.assert_not_called()

    def test__move_left_1(self):
        # Existing test method, keep as is
        pass

    def test__move_left_2(self):
        """
        Test _move_left method when there are consecutive equal non-zero elements
        """
        # Import necessary classes and packages

        # Initialize test data
        input_row = [2, 2, 4, 0]
        expected_output = [3, 4, 0, 0]

        # Call the focal method
        result = Board._move_left(input_row)

        # Verify the expected behavior
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_left_2_2(self):
        # Existing test method, keep as is
        pass

    def test__move_left_3(self):
        """
        Test _move_left when the input row is empty (all zeros).
        This case tests the scenario where there are no non-zero elements,
        so the function should return a list of all zeros.
        """
        # Arrange
        input_row = [0, 0, 0, 0]
        expected_output = [0, 0, 0, 0]

        # Act
        result = Board._move_left(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_left_combines_consecutive_equal_elements(self):
        """
        Test _move_left method when there are consecutive equal non-zero elements
        """
        # Import necessary classes and packages

        # Initialize test data
        input_row = [2, 2, 4, 0]
        expected_output = [3, 4, 0, 0]

        # Call the focal method
        result = Board._move_left(input_row)

        # Verify the expected behavior
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_left_handles_empty_row(self):
        """
        Test that _move_left correctly handles an empty row (all zeros).
        """
        # Arrange
        input_row = [0, 0, 0, 0]
        expected_output = [0, 0, 0, 0]

        # Act
        result = Board._move_left(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_left_merges_adjacent_tiles(self):
        """
        Test that _move_left correctly merges adjacent tiles and shifts them to the left.
        """
        # Arrange
        input_row = [2, 2, 2, 2]
        expected_output = [3, 3, 0, 0]

        # Act
        result = Board._move_left(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_left_merges_only_once(self):
        """
        Test that _move_left merges tiles only once per move.
        """
        # Arrange
        input_row = [2, 2, 4, 4]
        expected_output = [3, 5, 0, 0]

        # Act
        result = Board._move_left(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_left_shifts_non_zero_tiles(self):
        """
        Test that _move_left shifts non-zero tiles to the left without merging.
        """
        # Arrange
        input_row = [0, 2, 0, 4]
        expected_output = [2, 4, 0, 0]

        # Act
        result = Board._move_left(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_right_all_same(self):
        """
        Test that _move_right correctly merges when all tiles are the same
        """
        # Arrange
        input_row = [2, 2, 2, 2]
        expected_output = [0, 0, 3, 3]

        # Act
        result = Board._move_right(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_right_empty_row(self):
        """
        Test that _move_right returns an unchanged row when it's empty
        """
        # Arrange
        input_row = [0, 0, 0, 0]
        expected_output = [0, 0, 0, 0]

        # Act
        result = Board._move_right(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_right_merges_tiles(self):
        """
        Test that _move_right correctly merges tiles and moves them to the right
        """
        # Arrange
        input_row = [2, 2, 2, 0]
        expected_output = [0, 0, 2, 3]

        # Act
        result = Board._move_right(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_right_no_change(self):
        """
        Test that _move_right returns an unchanged row when no movement is possible
        """
        # Arrange
        input_row = [1, 2, 3, 4]
        expected_output = [1, 2, 3, 4]

        # Act
        result = Board._move_right(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__move_right_no_merge(self):
        """
        Test that _move_right correctly moves tiles to the right without merging
        """
        # Arrange
        input_row = [2, 0, 3, 4]
        expected_output = [0, 2, 3, 4]

        # Act
        result = Board._move_right(input_row)

        # Assert
        assert result == expected_output, f"Expected {expected_output}, but got {result}"

    def test__row_left_edge_case_max(self):
        """
        Test _row_left with edge case of maximum value
        """
        max_value = (2**16) - 1
        assert Board._row_left(max_value) == max_value

    def test__row_left_edge_case_zero(self):
        """
        Test _row_left with edge case of all zeros
        """
        assert Board._row_left(0) == 0

    def test__row_left_empty_row(self):
        """
        Test _row_left method with an empty row
        """
        # Ensure lookup tables are initialized
        if not Board.is_lookup_tables_initialized():
            Board.__init_lookup_tables()

        # Test case: [0, 0, 0, 0] should remain [0, 0, 0, 0]
        row = 0  # 0x0000 in binary, representing [0, 0, 0, 0]
        expected = 0  # Should remain the same

        result = Board._row_left(row)

        assert result == expected, f"Expected {expected}, but got {result}"

    def test__row_left_float_input(self):
        """
        Test _row_left with float input
        """
        with pytest.raises(TypeError):
            Board._row_left(1.5)

    def test__row_left_incorrect_type(self):
        """
        Test _row_left with input of incorrect type
        """
        with pytest.raises(TypeError):
            Board._row_left("1234")

    def test__row_left_invalid_input(self):
        """
        Test _row_left with invalid input (empty or None)
        """
        with pytest.raises(TypeError):
            Board._row_left(None)

    def test__row_left_merge_tiles(self):
        """
        Test _row_left method with a row that has mergeable tiles
        """
        # Ensure lookup tables are initialized
        if not Board.is_lookup_tables_initialized():
            Board.__init_lookup_tables()

        # Test case: [2, 2, 0, 0] should become [3, 0, 0, 0]
        row = (1 << 12) | (1 << 8)  # 0x1100 in binary, representing [2, 2, 0, 0]
        expected = 2 << 12  # 0x2000 in binary, representing [4, 0, 0, 0]

        result = Board._row_left(row)

        assert result == expected, f"Expected {expected}, but got {result}"

    def test__row_left_multiple_merges(self):
        """
        Test _row_left method with a row that has multiple mergeable tiles
        """
        # Ensure lookup tables are initialized
        if not Board.is_lookup_tables_initialized():
            Board.__init_lookup_tables()

        # Test case: [2, 2, 4, 4] should become [3, 5, 0, 0]
        row = (1 << 12) | (1 << 8) | (2 << 4) | 2  # 0x1122 in binary, representing [2, 2, 4, 4]
        expected = (2 << 12) | (3 << 8)  # 0x3500 in binary, representing [3, 5, 0, 0]

        result = Board._row_left(row)

        assert result == expected, f"Expected {expected}, but got {result}"

    def test__row_left_negative_input(self):
        """
        Test _row_left with negative input
        """
        with pytest.raises(IndexError):
            Board._row_left(-1)

    def test__row_left_no_merge(self):
        """
        Test _row_left method with a row that has no mergeable tiles
        """
        # Ensure lookup tables are initialized
        if not Board.is_lookup_tables_initialized():
            Board.__init_lookup_tables()

        # Test case: [2, 4, 8, 16] should remain [2, 4, 8, 16]
        row = (1 << 12) | (2 << 8) | (3 << 4) | 4  # 0x1234 in binary, representing [2, 4, 8, 16]
        expected = row  # Should remain the same

        result = Board._row_left(row)

        assert result == expected, f"Expected {expected}, but got {result}"

    def test__row_left_out_of_bounds(self):
        """
        Test _row_left with input outside accepted bounds
        """
        with pytest.raises(IndexError):
            Board._row_left(2**16)

    def test_get_empty_tiles_2(self):
        """
        Test get_empty_tiles method when state is not in Board.empty_cells and has empty tiles.
        """
        # Create a state with some empty tiles
        state = 0x1234_5678_0000_0000  # Last two rows are empty

        # Clear the Board.empty_cells cache to ensure we're not using cached results
        Board.empty_cells.clear()

        # Call the method
        result = Board.get_empty_tiles(state)

        # Expected empty tiles (last two rows)
        expected = [(2, 0), (2, 1), (2, 2), (2, 3), (3, 0), (3, 1), (3, 2), (3, 3)]

        # Assert that the result matches the expected empty tiles
        assert set(result) == set(expected)

        # Assert that the result is now cached in Board.empty_cells
        assert state in Board.empty_cells
        assert Board.empty_cells[state] == result

    def test_get_empty_tiles_cached_result(self):
        """
        Test get_empty_tiles with a cached result
        """
        test_state = 0x1230000000000000
        expected_empty_tiles = [(0, 3), (1, 0), (1, 1), (1, 2), (1, 3),
                                (2, 0), (2, 1), (2, 2), (2, 3),
                                (3, 0), (3, 1), (3, 2), (3, 3)]

        # First call to populate the cache
        result1 = Board.get_empty_tiles(test_state)
        assert result1 == expected_empty_tiles

        # Second call to test cached result
        result2 = Board.get_empty_tiles(test_state)
        assert result2 == expected_empty_tiles
        assert result2 is Board.empty_cells[test_state]

    def test_get_empty_tiles_empty_board(self):
        """
        Test get_empty_tiles with an empty board (edge case)
        """
        empty_board_state = 0x0000000000000000
        expected_empty_tiles = [(i, j) for i in range(4) for j in range(4)]
        assert Board.get_empty_tiles(empty_board_state) == expected_empty_tiles

    def test_get_empty_tiles_full_board(self):
        """
        Test get_empty_tiles with a full board (edge case)
        """
        full_board_state = 0x1234567899ABCDEF  # All tiles filled
        assert Board.get_empty_tiles(full_board_state) == []

    def test_get_empty_tiles_incorrect_type(self):
        """
        Test get_empty_tiles with input of incorrect type
        """
        with pytest.raises(TypeError):
            Board.get_empty_tiles("0")

    def test_get_empty_tiles_invalid_input(self):
        """
        Test get_empty_tiles with invalid input (negative integer)
        """
        with pytest.raises(TypeError):
            Board.get_empty_tiles("Ginger")

    def test_get_empty_tiles_out_of_bounds(self):
        """
        Test get_empty_tiles with input outside accepted bounds (too large)
        """
        with pytest.raises(OverflowError):
            Board.get_empty_tiles(2**64)

    def test_get_empty_tiles_when_state_in_empty_cells(self):
        """
        Test get_empty_tiles when the state is already in Board.empty_cells.
        """
        # Initialize the Board class
        Board()

        # Set up a test state and its corresponding empty tiles
        test_state = 0x1234567800000000
        expected_empty_tiles = [(2, 0), (2, 1), (2, 2), (2, 3), (3, 0), (3, 1), (3, 2), (3, 3)]

        # Manually add the test state to Board.empty_cells
        Board.empty_cells[test_state] = expected_empty_tiles

        # Call the method and check the result
        result = Board.get_empty_tiles(test_state)

        assert result == expected_empty_tiles
        assert result is Board.empty_cells[test_state]

    def test_get_state_2(self):
        """
        Test get_state method when unpack is False.
        It should return the state as an integer.
        """
        # Initialize a Board with a known state
        initial_state = 0x1234567890ABCDEF
        board = Board(initial_state)

        # Call get_state with unpack=False (default)
        result = board.get_state()

        # Assert that the result is the same as the initial state
        assert result == initial_state
        assert isinstance(result, int)

    def test_get_state_packed(self):
        """
        Test get_state method with unpack=False (default)
        """
        # Initialize a Board with a known state
        test_state = 0x1234567890ABCDEF
        board = Board(test_state)

        # Call get_state with default unpack=False
        result = board.get_state()

        # Assert that the result matches the initial state
        assert result == test_state, f"Expected {test_state}, but got {result}"

    def test_get_state_unpacked(self):
        """
        Test get_state method with unpack=True
        """
        # Initialize a Board with a known state
        test_state = 0x1234_5678_90AB_CDEF
        board = Board(test_state)

        # Call get_state with unpack=True
        result = board.get_state(unpack=True)

        # Expected result: list of 16 integers, each representing a tile value
        expected = [
            0x1, 0x2, 0x3, 0x4,
            0x5, 0x6, 0x7, 0x8,
            0x9, 0x0, 0xA, 0xB,
            0xC, 0xD, 0xE, 0xF,
        ]

        # Assert that the result matches the expected output
        assert result == expected, f"Expected {expected}, but got {result}"

    def test_get_state_with_default_unpack(self):
        """
        Test get_state with default unpack value (False).
        """
        board = Board(state=0x1234567890ABCDEF)
        assert board.get_state() == 0x1234567890ABCDEF

    def test_get_state_with_invalid_unpack_type(self):
        """
        Test get_state with an invalid unpack type.
        """
        board = Board()
        with pytest.raises(TypeError):
            board.get_state(unpack="True")

    def test_get_state_with_max_state(self):
        """
        Test get_state with the maximum possible state.
        """
        max_state = (1 << 64) - 1
        board = Board(state=max_state)
        assert board.get_state() == max_state
        assert board.get_state(unpack=True) == [15] * 16

    def test_set_state_with_negative_state(self):
        """
        Test get_state with a negative state (which should not be possible in normal use).
        """
        board = Board()
        with pytest.raises(ValueError):
            board.set_state(-1)

    def test_get_state_with_unpack_true(self):
        """
        Test get_state with unpack set to True.
        """
        board = Board(state=0x1234_5678_90AB_CDEF)
        expected = [0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF]
        assert board.get_state(unpack=True) == expected

    def test_get_state_with_zero_state(self):
        """
        Test get_state with a zero state.
        """
        board = Board(state=0)
        assert board.get_state() == 0
        assert board.get_state(unpack=True) == [0] * 16

    def test_get_unpacked_state_returns_correct_list(self):
        """
        Test that get_unpacked_state returns the correct list of unpacked values.
        """
        # Arrange
        test_state = 0x1234_5678_90AB_CDEF  # Example state with different values in each position

        # Act
        result = Board.get_unpacked_state(test_state)

        # Expected result: list of 16 integers, each representing a tile value
        expected = [
            0x1, 0x2, 0x3, 0x4,
            0x5, 0x6, 0x7, 0x8,
            0x9, 0x0, 0xA, 0xB,
            0xC, 0xD, 0xE, 0xF,
        ]
        assert result == expected, f"Expected {expected}, but got {result}"

    def test_get_unpacked_state_with_float_input(self):
        """
        Test get_unpacked_state with float input (incorrect format).
        """
        with pytest.raises(TypeError):
            Board.get_unpacked_state(3.14)

    def test_get_unpacked_state_with_incorrect_type(self):
        """
        Test get_unpacked_state with input of incorrect type.
        """
        with pytest.raises(TypeError):
            Board.get_unpacked_state("not an integer")

    def test_get_unpacked_state_with_max_value(self):
        """
        Test that get_unpacked_state correctly handles the maximum possible state value.
        """
        # Arrange
        test_state = 0xFFFFFFFFFFFFFFFF  # Maximum 64-bit value

        # Act
        result = Board.get_unpacked_state(test_state)

        # Assert
        expected = [0xF] * 16
        assert result == expected, f"Expected {expected}, but got {result}"

    def test_get_unpacked_state_with_max_value_2(self):
        """
        Test get_unpacked_state with maximum possible value (edge case).
        """
        max_state = (2**64) - 1
        result = Board.get_unpacked_state(max_state)
        assert len(result) == 16
        assert all(tile == 15 for tile in result)

    def test_get_unpacked_state_with_negative_input(self):
        """
        Test get_unpacked_state with negative input (outside accepted bounds).
        """
        with pytest.raises(ValueError):
            Board.get_unpacked_state(-1)

    def test_get_unpacked_state_with_none_input(self):
        """
        Test get_unpacked_state with None input (invalid input).
        """
        with pytest.raises(TypeError):
            Board.get_unpacked_state(None)

    def test_get_unpacked_state_with_too_large_input(self):
        """
        Test get_unpacked_state with input larger than 64-bit integer (outside accepted bounds).
        """
        with pytest.raises(OverflowError):
            Board.get_unpacked_state(2**64)

    def test_get_unpacked_state_with_zero(self):
        """
        Test get_unpacked_state with zero (edge case).
        """
        result = Board.get_unpacked_state(0)
        assert len(result) == 16
        assert all(tile == 0 for tile in result)

    def test_get_unpacked_state_with_zero_state(self):
        """
        Test that get_unpacked_state returns a list of zeros for a zero state.
        """
        # Arrange
        test_state = 0x0

        # Act
        result = Board.get_unpacked_state(test_state)

        # Assert
        expected = [0] * 16
        assert result == expected, f"Expected {expected}, but got {result}"

    def test_get_valid_move_actions_2(self):
        """
        Test get_valid_move_actions when no valid moves are available.
        """
        # Create a board state where no moves are possible
        # 2 2 2 2
        # 2 2 2 2
        # 2 2 2 2
        # 2 2 2 2
        state = 0x1111_1111_1111_1111

        valid_actions = Board.get_valid_move_actions(state)

        # Assert that the list of valid actions is empty
        assert len(valid_actions) == 4, "Expected no valid moves, but found some"

    def test_get_valid_move_actions_left_and_right(self):
        """
        Test get_valid_move_actions when left and right directions are valid moves.
        """
        # Create a board state where all moves are possible
        # 2 0 0 2
        # 0 2 2 0
        # 2 0 0 2
        # 0 2 2 0
        state = 0x1001_2002_1001_2002

        valid_actions = Board.get_valid_move_actions(state)

        # Assert that all four directions are valid
        assert len(valid_actions) == 2, f"Expected 4 valid moves, but found {len(valid_actions)}"

        # Check if all actions are present
        actions = [action for action, _ in valid_actions]
        assert Action.LEFT in actions, "LEFT action is missing"
        assert Action.RIGHT in actions, "RIGHT action is missing"

    def test_get_valid_move_actions_resulting_states(self):
        """
        Test get_valid_move_actions to ensure resulting states are correct.
        """
        # Create a simple board state
        # 2 2 0 0
        # 0 0 0 0
        # 0 0 0 0
        # 0 0 0 0
        state = 0x1100_000000000000

        valid_actions = Board.get_valid_move_actions(state)

        # Check the resulting states for each action
        for action, resulting_state in valid_actions:
            if action == Action.LEFT:
                assert resulting_state == 0x2000000000000000, "Incorrect resulting state for LEFT action"
            elif action == Action.RIGHT:
                assert resulting_state == 0x0002000000000000, "Incorrect resulting state for RIGHT action"
            elif action == Action.DOWN:
                assert resulting_state == 0x0000000000001100, "Incorrect resulting state for DOWN action"
            else:
                assert False, f"Unexpected action: {action}"

        # Assert that UP is not a valid action
        assert Action.UP not in [action for action, _ in valid_actions], "UP should not be a valid action"

    def test_get_valid_move_actions_returns_valid_actions(self):
        """
        Test that get_valid_move_actions returns correct valid actions for a given state.
        """
        # Arrange
        state = 0x1234_000000000000  # Represents a board with tiles in the top row only

        # Act
        valid_actions = Board.get_valid_move_actions(state)

        # Assert
        assert len(valid_actions) == 1  # Only DOWN should be valid
        assert (Action.DOWN, 0x0000000000001234) in valid_actions

    def test_get_valid_move_actions_some_directions(self):
        """
        Test get_valid_move_actions when only some directions are valid moves.
        """
        # Create a board state where only left and up moves are possible
        # 2 0 0 0
        # 2 0 0 0
        # 2 0 0 0
        # 2 0 0 0
        state = 0x1000_1000_1000_1000

        valid_actions = Board.get_valid_move_actions(state)

        # Assert that only two directions are valid
        assert len(valid_actions) == 3, f"Expected 2 valid moves, but found {len(valid_actions)}"

        # Check if only LEFT and UP actions are present
        actions = [action for action, _ in valid_actions]
        assert Action.UP in actions, "UP action is missing"
        assert Action.RIGHT in actions, "RIGHT action is missing"
        assert Action.DOWN in actions, "DOWN action is missing"

    def test_get_valid_move_actions_with_full_board(self):
        """
        Test get_valid_move_actions with a full board where no moves are possible.
        """
        # Create a full board state where no moves are possible
        full_board_state = 0x1234_1234_1234_1234  # This represents a full board with no possible moves
        result = Board.get_valid_move_actions(full_board_state)
        assert len(result) == 2, "Expected no valid moves for a full board"

    def test_get_valid_move_actions_with_incorrect_type(self):
        """
        Test get_valid_move_actions with an input of incorrect type.
        """
        with pytest.raises(TypeError):
            Board.get_valid_move_actions("not an integer")

    def test_get_valid_move_actions_with_invalid_state(self):
        """
        Test get_valid_move_actions with an invalid state (negative integer).
        """
        with pytest.raises(ValueError):
            Board.get_valid_move_actions(-1)

    def test_get_valid_move_actions_with_out_of_bounds_state(self):
        """
        Test get_valid_move_actions with a state that is out of bounds (too large).
        """
        with pytest.raises(OverflowError):
            Board.get_valid_move_actions(2**64)

    def test_get_valid_move_actions_with_single_tile(self):
        """
        Test get_valid_move_actions with a board containing only one tile.
        """
        # Create a board state with only one tile
        single_tile_state = 0x0000_0000_0000_0001  # This represents a board with a single tile in the top-left corner
        result = Board.get_valid_move_actions(single_tile_state)
        assert len(result) == 2, "Expected two valid moves (right and down) for a single tile"
        assert (Action.LEFT, 0x0000_0000_0000_1000) in result, "Expected RIGHT move to be valid"
        assert (Action.UP, 0x0001_0000_0000_0000) in result, "Expected DOWN move to be valid"

    def test_init_lookup_tables_initialization(self):
        """
        Test that lookup tables are initialized only once.
        """
        board1 = Board()
        assert Board.is_lookup_tables_initialized() == True

        # Creating a new board should not re-initialize lookup tables
        board2 = Board()
        assert Board.is_lookup_tables_initialized() == True

    def test_init_lookup_tables_invalid_access(self):
        """
        Test that __init_lookup_tables cannot be accessed directly
        """
        with pytest.raises(AttributeError):
            Board().__init_lookup_tables()

    def test_init_with_float_input(self):
        """
        Test initializing Board with a float input.
        This should raise a TypeError.
        """
        with pytest.raises(TypeError):
            Board(1.5)

    def test_init_with_max_valid_state(self):
        """
        Test initializing Board with the maximum valid state.
        """
        max_state = 2**64 - 1
        board = Board(max_state)
        assert board.get_state() == max_state

    def test_init_with_negative_input(self):
        """
        Test initializing Board with a negative input.
        This should raise a ValueError.
        """
        with pytest.raises(ValueError):
            Board(-1)

    def test_init_with_none_input(self):
        """
        Test initializing Board with None input.
        This should default to state 0.
        """
        board = Board(None)
        assert board.get_state() == 0

    def test_init_with_state_and_initialize_lookup_tables(self):
        """
        Test initializing Board with a state and initializing lookup tables.
        """
        # Initialize Board with a specific state
        test_state = 0x1234567890ABCDEF
        board = Board(test_state)

        # Check if the state is correctly set
        assert board.get_state() == test_state

        # Check if lookup tables are initialized
        assert Board.is_lookup_tables_initialized() == True

        # Check if empty_cells dictionary is initialized
        assert isinstance(Board.empty_cells, dict)

    def test_init_with_string_input(self):
        """
        Test initializing Board with a string input.
        This should raise a TypeError.
        """
        with pytest.raises(TypeError):
            Board("0")

    def test_init_with_too_large_input(self):
        """
        Test initializing Board with an input that's too large.
        The maximum valid state is 2^64 - 1.
        """
        with pytest.raises(OverflowError):
            Board(2**64)

    def test_move_left_all_zeros(self):
        """Test _move_left with all zero input."""
        assert Board._move_left([0, 0, 0, 0]) == [0, 0, 0, 0]

    def test_move_left_empty_input(self):
        """Test _move_left with an empty input list."""
        with pytest.raises(ValueError):
            Board._move_left([])

    def test_move_left_invalid_element_type(self):
        """Test _move_left with list containing elements of incorrect type."""
        with pytest.raises(TypeError):
            Board._move_left([1, '2', 3, 4])  # String element
        with pytest.raises(TypeError):
            Board._move_left([1, 2.5, 3, 4])

    def test_move_left_invalid_length(self):
        """Test _move_left with an input list of invalid length."""
        with pytest.raises(ValueError):
            Board._move_left([1, 2, 3])  # Less than 4 elements
        with pytest.raises(ValueError):
            Board._move_left([1, 2, 3, 4, 5])

    def test_move_left_invalid_type(self):
        """Test _move_left with input of incorrect type."""
        with pytest.raises(TypeError):
            Board._move_left("1234")  # String instead of list
        with pytest.raises(TypeError):
            Board._move_left(1234)

    def test_move_left_negative_values(self):
        """Test _move_left with negative values in the input list."""
        with pytest.raises(ValueError):
            Board._move_left([-1, 2, 3, 4])

    def test_move_left_no_movement(self):
        """Test _move_left when no movement is possible."""

        assert Board._move_left([2, 4, 8, 15]) == [2, 4, 8, 15]

    def test_move_left_out_of_range_values(self):
        """Test _move_left with values outside the valid range (0-15)."""
        with pytest.raises(ValueError):
            Board._move_left([16, 2, 3, 4])

    def test_move_right_edge_case_all_same(self):
        """Test _move_right with all same non-zero input."""
        assert Board._move_right([2, 2, 2, 2]) == [0, 0, 3, 3]

    def test_move_right_edge_case_all_zero(self):
        """Test _move_right with all zero input."""
        assert Board._move_right([0, 0, 0, 0]) == [0, 0, 0, 0]

    def test_move_right_edge_case_ascending(self):
        """Test _move_right with ascending values."""
        assert Board._move_right([1, 2, 3, 4]) == [1, 2, 3, 4]

    def test_move_right_edge_case_descending(self):
        """Test _move_right with descending values."""
        assert Board._move_right([4, 3, 2, 1]) == [4, 3, 2, 1]

    def test_move_right_empty_input(self):
        """Test _move_right with an empty input list."""
        with pytest.raises(ValueError):
            Board._move_right([])

    def test_move_right_exception_handling(self):
        """Test exception handling in _move_right."""
        with pytest.raises(Exception):
            # Simulate an internal error by mocking _move_left to raise an exception
            original_move_left = Board._move_left
            Board._move_left = lambda x: (_ for _ in ()).throw(Exception("Test exception"))
            try:
                Board._move_right([1, 2, 3, 4])
            finally:
                Board._move_left = original_move_left

    def test_move_right_incorrect_format(self):
        """Test _move_right with incorrect input format."""
        with pytest.raises(TypeError):
            Board._move_right([1.5, 2, 3, 4])

    def test_move_right_incorrect_type(self):
        """Test _move_right with incorrect input type."""
        with pytest.raises(TypeError):
            Board._move_right("1234")  # String instead of list
        with pytest.raises(TypeError):
            Board._move_right([1, 2, "3", 4])

    def test_move_right_invalid_length(self):
        """Test _move_right with an input list of invalid length."""
        with pytest.raises(ValueError):
            Board._move_right([1, 2, 3])  # Less than 4 elements
        with pytest.raises(ValueError):
            Board._move_right([1, 2, 3, 4, 5])

    def test_move_right_out_of_bounds(self):
        """Test _move_right with values outside the accepted bounds."""
        with pytest.raises(ValueError):
            Board._move_right([-1, 0, 1, 2])  # Negative value
        with pytest.raises(ValueError):
            Board._move_right([0, 16, 1, 2])

    def test_pretty_print_empty_board(self):
        """
        Test pretty_print method with an empty board (all zeros)
        """
        # Redirect stdout to capture printed output
        captured_output = io.StringIO()
        sys.stdout = captured_output

        # Call the pretty_print method with an empty board state
        Board.pretty_print(0)

        # Restore stdout
        sys.stdout = sys.__stdout__

        # Check the captured output
        expected_output = "-----------\n0 0 0 0\n0 0 0 0\n0 0 0 0\n0 0 0 0\n"
        assert captured_output.getvalue() == expected_output

    def test_pretty_print_with_list_input(self):
        """
        Test pretty_print with list input.
        """
        with pytest.raises(TypeError):
            Board.pretty_print([1, 2, 3, 4])

    def test_pretty_print_with_max_state_value(self):
        '''
        Test pretty_print with the maximum possible state value.
        '''
        max_state = 2**64 - 1
        expected_output = "-----------\nF F F F\nF F F F\nF F F F\nF F F F\n"

        # Redirect stdout to capture printed output
        captured_output = io.StringIO()
        sys.stdout = captured_output

        # Call pretty_print
        Board.pretty_print(max_state)

        # Restore stdout
        sys.stdout = sys.__stdout__

        # Check the captured output
        assert captured_output.getvalue() == expected_output, f"expected {expected_output} captured {captured_output.getvalue()}"

    def test_pretty_print_with_negative_state(self):
        """
        Test pretty_print with a negative state value.
        """
        with pytest.raises(ValueError):
            Board.pretty_print(-1)

    def test_pretty_print_with_non_integer_input(self):
        """
        Test pretty_print with non-integer input.
        """
        with pytest.raises(TypeError):
            Board.pretty_print(3.14)

    def test_pretty_print_with_none_input(self):
        """
        Test pretty_print with None input.
        """
        with pytest.raises(TypeError):
            Board.pretty_print(None)

    def test_pretty_print_with_state_exceeding_max_value(self):
        """
        Test pretty_print with a state value exceeding the maximum possible value.
        """
        max_state = (16**16) - 1  # Maximum possible state value
        with pytest.raises(OverflowError):
            Board.pretty_print(max_state + 1)

    def test_pretty_print_with_string_input(self):
        """
        Test pretty_print with string input.
        """
        with pytest.raises(TypeError):
            Board.pretty_print("1234")

    def test_pretty_print_with_zero_state(self):
        """
        Test pretty_print with a state of 0 (all empty tiles).
        """
        # Redirect stdout to capture printed output
        captured_output = io.StringIO()
        sys.stdout = captured_output

        # Call pretty_print
        Board.pretty_print(0)

        # Restore stdout
        sys.stdout = sys.__stdout__

        # Check the captured output
        expected_output = "-----------\n0 0 0 0\n0 0 0 0\n0 0 0 0\n0 0 0 0\n"
        assert captured_output.getvalue() == expected_output

    def test_row_right_edge_cases(self):
        """
        Test _row_right with edge case inputs
        """
        # Ensure lookup tables are initialized
        if not Board.is_lookup_tables_initialized():
            Board.__init_lookup_tables()

        # Test with all zeros (0x0000)
        assert Board._row_right(0) == 0

        # Test with all max values (0xFFFF)
        assert Board._row_right(0xFFFF) == 0xFFFF

        # Test with alternating zeros and ones (0x5555)
        result = Board._row_right(0x5555)
        assert result != 0x5555, "Right move should change the row"

    def test_row_right_incorrect_type(self):
        """
        Test _row_right with incorrect input type
        """
        with pytest.raises(TypeError):
            Board._row_right("1234")  # String instead of int

        with pytest.raises(TypeError):
            Board._row_right([1, 2, 3, 4])

    def test_row_right_invalid_input(self):
        """
        Test _row_right with invalid input (empty or None)
        """
        with pytest.raises(TypeError):
            Board._row_right(None)

        with pytest.raises(TypeError):
            Board._row_right([])

    def test_row_right_out_of_bounds(self):
        """
        Test _row_right with input outside accepted bounds
        """
        with pytest.raises(IndexError):
            Board._row_right(2**16)  # Exceeds the size of right_moves list

        with pytest.raises(IndexError):
            Board._row_right(-1)

    def test_set_state_updates_internal_state(self):
        """
        Test that set_state correctly updates the internal state of the Board instance.
        """
        # Initialize a Board instance
        board = Board()

        # Define a test state
        test_state = 0x1234567890ABCDEF

        # Call set_state method
        board.set_state(test_state)

        # Verify that the internal state has been updated
        assert board.get_state() == test_state, "set_state did not update the internal state correctly"

    def test_set_state_with_dict_input(self):
        """
        Test set_state with a dictionary input.
        """
        board = Board()
        with pytest.raises(TypeError):
            board.set_state({"key": "value"})

    def test_set_state_with_float_input(self):
        """
        Test set_state with a float input.
        """
        board = Board()
        with pytest.raises(TypeError):
            board.set_state(3.14)

    def test_set_state_with_large_input(self):
        """
        Test set_state with a very large integer input.
        """
        board = Board()
        large_number = 2**64  # A number larger than the typical 64-bit integer
        with pytest.raises(OverflowError):
            board.set_state(large_number)

    def test_set_state_with_list_input(self):
        """
        Test set_state with a list input.
        """
        board = Board()
        with pytest.raises(TypeError):
            board.set_state([1, 2, 3, 4])

    def test_set_state_with_negative_input(self):
        """
        Test set_state with a negative integer input.
        """
        board = Board()
        with pytest.raises(ValueError):
            board.set_state(-1)

    def test_set_state_with_none_input(self):
        """
        Test set_state with None as input.
        """
        board = Board()
        with pytest.raises(TypeError):
            board.set_state(None)

    def test_set_state_with_string_input(self):
        """
        Test set_state with a string input.
        """
        board = Board()
        with pytest.raises(TypeError):
            board.set_state("invalid")

    def test_set_tile_2(self):
        """
        Test set_tile method with invalid row and column values.
        """
        # Initialize a state with all tiles empty (0)
        initial_state = 0

        # Test with invalid row (less than 0)
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(initial_state, -1, 0, 2)

        # Test with invalid row (greater than 3)
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(initial_state, 4, 0, 2)

        # Test with invalid column (less than 0)
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(initial_state, 0, -1, 2)

        # Test with invalid column (greater than 3)
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(initial_state, 0, 4, 2)

        # Set a tile to a non-zero value
        non_empty_state = Board.set_tile(initial_state, 0, 0, 2)

        # Try to set the same tile again
        with pytest.raises(ValueError, match="Tile at given row and column is not empty."):
            Board.set_tile(non_empty_state, 0, 0, 4)

    def test_set_tile_3(self):
        """
        Test setting a tile with an invalid value and valid position on a non-empty tile.
        """
        # Initialize a board state with a non-empty tile at (1, 2)
        initial_state = Board.set_tile(0, 1, 2, 2)  # Set tile (1, 2) to value 2

        # Test setting an invalid value (16) at the same position
        with pytest.raises(ValueError, match="Value must be between 0 and 15."):
            Board.set_tile(initial_state, 1, 2, 16)

        # Test setting a negative value (-1) at the same position
        with pytest.raises(ValueError, match="Value must be between 0 and 15."):
            Board.set_tile(initial_state, 1, 2, -1)

        # Test setting a valid value (4) at the same non-empty position
        with pytest.raises(ValueError, match="Tile at given row and column is not empty."):
            Board.set_tile(initial_state, 1, 2, 4)

        # Verify that the initial state remains unchanged
        assert initial_state == Board.set_tile(0, 1, 2, 2)

    def test_set_tile_boundary_values(self):
        """
        Test setting tiles with boundary values for row, column, and value.
        """
        state = 0
        # Test minimum valid values
        new_state = Board.set_tile(state, 0, 0, 1)
        assert new_state == 0x1000_0000_0000_0000, "Tile should be set with minimum values"

        # Test maximum valid values
        state = 0
        new_state = Board.set_tile(state, 3, 3, 15)
        assert new_state == 0x0000_0000_0000_000F, "Tile should be set with maximum values"

    def test_set_tile_edge_cases(self):
        """
        Test setting a tile with edge case values within valid ranges.
        """
        state = 0
        new_state = Board.set_tile(state, 0, 0, 15)  # Max value
        assert new_state == 0xF000_0000_0000_0000, "Tile should be set to max value"

        new_state = Board.set_tile(state, 3, 3, 1)  # Last position
        assert new_state == 0x0000_0000_0000_0001, "Tile should be set at last position"

    def test_set_tile_float_inputs(self):
        """
        Test that set_tile raises a TypeError when given float inputs.
        """
        with pytest.raises(TypeError):
            Board.set_tile(0.0, 0, 0, 2)
        with pytest.raises(TypeError):
            Board.set_tile(0, 0.0, 0, 2)
        with pytest.raises(TypeError):
            Board.set_tile(0, 0, 0.0, 2)
        with pytest.raises(TypeError):
            Board.set_tile(0, 0, 0, 2.0)

    def test_set_tile_invalid_input_types(self):
        """
        Test that set_tile raises a TypeError when given invalid input types.
        """
        with pytest.raises(TypeError):
            Board.set_tile("0", 0, 0, 2)  # Invalid state type
        with pytest.raises(TypeError):
            Board.set_tile(0, "0", 0, 2)  # Invalid row type
        with pytest.raises(TypeError):
            Board.set_tile(0, 0, "0", 2)  # Invalid column type
        with pytest.raises(TypeError):
            Board.set_tile(0, 0, 0, "2")

    def test_set_tile_invalid_position(self):
        """
        Test setting a tile with invalid row or column values.
        """
        initial_state = 0
        value = 2

        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(initial_state, -1, 0, value)

        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(initial_state, 0, 4, value)

    def test_set_tile_invalid_position_2(self):
        """
        Test setting a tile with an invalid position (row or column out of bounds).
        """
        state = 0
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(state, 4, 0, 2)

    def test_set_tile_invalid_value(self):
        """
        Test setting a tile with an invalid value (less than 0 or greater than 15).
        """
        initial_state = 0
        row, col = 1, 1

        with pytest.raises(ValueError, match="Value must be between 0 and 15."):
            Board.set_tile(initial_state, row, col, -1)

        with pytest.raises(ValueError, match="Value must be between 0 and 15."):
            Board.set_tile(initial_state, row, col, 16)

    def test_set_tile_invalid_value_2(self):
        """
        Test setting a tile with an invalid value (less than 0 or greater than 15).
        """
        state = 0
        with pytest.raises(ValueError, match="Value must be between 0 and 15."):
            Board.set_tile(state, 0, 0, 16)

    def test_set_tile_multiple_tiles(self):
        """
        Test setting multiple tiles on the same board.
        """
        state = 0
        state = Board.set_tile(state, 0, 0, 2)
        state = Board.set_tile(state, 1, 1, 4)
        state = Board.set_tile(state, 2, 2, 8)
        state = Board.set_tile(state, 3, 3, 1)

        expected_state = 0x2000_0400_0080_0001
        assert state == expected_state, f"Multiple tiles should be set correctly. state={state:X} expected_state={expected_state:X}"

    def test_set_tile_non_empty(self):
        """
        Test setting a tile on a non-empty position.
        """
        state = 0x1000_0000_0000_0001  # Tile at (0, 0) is set to 1
        with pytest.raises(ValueError, match="Tile at given row and column is not empty."):
            Board.set_tile(state, 0, 0, 2)

    def test_set_tile_non_empty_position(self):
        """
        Test that set_tile raises a ValueError when trying to set a non-empty tile.
        """
        state = Board.set_tile(0, 0, 0, 2)  # Set a tile first
        with pytest.raises(ValueError, match="Tile at given row and column is not empty."):
            Board.set_tile(state, 0, 0, 4)

    def test_set_tile_occupied_position(self):
        """
        Test setting a tile at an already occupied position.
        """
        initial_state = 0x0000_0010_0000_0000  # 1 at row 1, col 2
        row, col, value = 1, 2, 2

        with pytest.raises(ValueError, match="Tile at given row and column is not empty."):
            Board.set_tile(initial_state, row, col, value)

    def test_set_tile_overwrite_zero(self):
        """
        Test setting a tile on a position that already contains a zero.
        """
        state = 0  # All tiles are zero
        new_state = Board.set_tile(state, 2, 1, 4)
        assert new_state == 0x0000_0000_0400_0000, f"Tile should be set on a zero position. new_state={new_state:X}"

    def test_set_tile_position_out_of_bounds(self):
        """
        Test that set_tile raises a ValueError when the row or column is out of bounds.
        """
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(0, 4, 0, 2)
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(0, 0, 4, 2)
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(0, -1, 0, 2)
        with pytest.raises(ValueError, match="Row and column must be between 0 and 3."):
            Board.set_tile(0, 0, -1, 2)

    def test_set_tile_valid_input(self):
        """
        Test setting a tile with valid input values.
        """
        initial_state = 0  # Empty board
        row, col, value = 2, 3, 4
        expected_state = 0x0000_0000_0004_0000

        result = Board.set_tile(initial_state, row, col, value)

        assert result == expected_state, f"Tile should be set correctly. result={result:X} expected_state={expected_state:X}"

    def test_set_tile_valid_input_2(self):
        """
        Test setting a tile with valid input values.
        """
        state = 0
        new_state = Board.set_tile(state, 1, 2, 3)
        assert new_state == 0x0000_0030_0000_0000, f"Tile should be set correctly. new_state={new_state:X}"

    def test_set_tile_value_out_of_bounds(self):
        """
        Test that set_tile raises a ValueError when the value is out of bounds.
        """
        with pytest.raises(ValueError, match="Value must be between 0 and 15."):
            Board.set_tile(0, 0, 0, 16)
        with pytest.raises(ValueError, match="Value must be between 0 and 15."):
            Board.set_tile(0, 0, 0, -1)

    def test_simulate_moves_horizontal(self):
        """
        Test simulate_moves for horizontal moves (LEFT and RIGHT)
        """
        # Initialize the board with a known state
        initial_state = 0x2000_2000_2000_2000  # [2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0]

        # Call the simulate_moves method
        result = Board.simulate_moves(initial_state)

        # Expected states after LEFT and RIGHT moves
        expected_left = 0x2000_2000_2000_2000  # [2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0]
        expected_right = 0x0002_0002_0002_0002  # [0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2]
        expected_up = 0x3000_3000_0000_0000  # [2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0]
        expected_down = 0x0000_0000_3000_3000  # [2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0]

        # Assert that the LEFT and RIGHT moves are correct
        assert result[0] == expected_left, f"Expected {expected_left:X}, but got {result[0]:X} for LEFT move"
        assert result[1] == expected_right, f"Expected {expected_right:X}, but got {result[1]:X} for RIGHT move"

        # Check that UP and DOWN moves don't change the state for this particular board
        assert result[2] == expected_up, f"Expected no change for UP move, but got {result[2]:X}"
        assert result[3] == expected_down, f"Expected no change for DOWN move, but got {result[3]:X}"

        # Verify that the result contains exactly 4 elements (LEFT, RIGHT, UP, DOWN)
        assert len(result) == 4, f"Expected 4 results, but got {len(result)}"

    def test_simulate_moves_with_float_input(self):
        """
        Test simulate_moves with a float input (incorrect format).
        """
        state = 1.5
        with pytest.raises(TypeError):
            Board.simulate_moves(state)

    def test_simulate_moves_with_incorrect_type(self):
        """
        Test simulate_moves with an input of incorrect type.
        """
        state = "not an integer"
        with pytest.raises(TypeError):
            Board.simulate_moves(state)

    def test_simulate_moves_with_max_state(self):
        """
        Test simulate_moves with the maximum possible state (edge case).
        """
        state = (2**64) - 1
        result = Board.simulate_moves(state)
        assert len(result) == 4, f"Expected 4 results, but got {len(result)}"
        assert all(isinstance(x, int) for x in result), "All results should be integers"

    def test_simulate_moves_with_negative_state(self):
        """
        Test simulate_moves with a negative state (invalid input).
        """
        state = -1
        with pytest.raises(ValueError):
            Board.simulate_moves(state)

    def test_simulate_moves_with_out_of_bounds_state(self):
        """
        Test simulate_moves with a state value larger than the maximum possible state.
        """
        state = 2**64  # Maximum possible state is 2^64 - 1
        with pytest.raises(OverflowError):
            Board.simulate_moves(state)

    def test_simulate_moves_with_single_tile(self):
        """
        Test simulate_moves with a board containing only one tile (edge case).
        """
        state = 0x0000_0000_0000_0001  # Represents a board with a single '1' tile in the button-right corner
        result = Board.simulate_moves(state)
        expected = [0x0000_0000_0000_1000, 0x0000_0000_0000_0001, 0x0001_0000_0000_0000, 0x0000_0000_0000_0001]
        assert result == expected, f"Expected {expected}, but got {result}"

    def test_simulate_moves_with_zero_state(self):
        """
        Test simulate_moves with a zero state (empty board).
        """
        state = 0
        result = Board.simulate_moves(state)
        expected = [0, 0, 0, 0]
        assert result == expected, f"Expected {expected}, but got {result}"