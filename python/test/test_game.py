from src.board import Board
from src.game import Game2048
from src.interfaces import CLI2048
from src.players import Player, RandomPlayer
from unittest.mock import MagicMock
import pytest
import random

# Add this fixture to clear global state before each test
@pytest.fixture(autouse=True)
def reset_board_empty_cells():
    Board.reset()

class TestGame:

    def test___init___2(self):
        """
        Test initialization of Game2048 with custom board and player
        """
        # Create a custom board
        custom_board = Board()
        custom_board.set_state(0x1234567890ABCDEF)  # Set a specific state

        # Create a custom player
        class CustomPlayer(Player):
            def choose_action(self, valid_actions):
                return valid_actions[0]

        custom_player = CustomPlayer()

        # Initialize the game with custom board and player
        game = Game2048(board=custom_board, player=custom_player, move_count=10)

        # Assert that the game's board is the custom board
        assert game.board == custom_board

        # Assert that the game's player is the custom player
        assert game.player == custom_player

        # Assert that the move count is set correctly
        assert game.move_count == 10

        # Assert that the board has the correct number of tiles
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert len(empty_tiles) <= 14

        # Assert that the interface is None
        assert game.interface is None

    def test_add_random_tile_2(self):
        """
        Test that add_random_tile adds a tile to a non-full board
        """
        # Create a game with a custom board that has empty tiles
        initial_state = 0x1234567800000000  # Board with 3 empty tiles
        board = Board()
        board.set_state(initial_state)
        game = Game2048(board=board)

        # Mock random.choice to return a specific empty tile
        random.choice = lambda x: (3, 3)

        # Mock random.random to always return 0.5 (less than 0.9)
        random.random = lambda: 0.5

        # Call the method under test
        game.add_random_tile()

        # Get the new state
        new_state = game.board.get_state()

        # Check that a tile was added
        assert new_state != initial_state

        # Check that the added tile is in the correct position (3, 3) and has value 1
        assert (new_state & 0xF) == 1  # The last 4 bits should be 1 (2^1)

        # Check that no other tiles were changed
        assert (new_state >> 4) == (initial_state >> 4)

    def test_add_random_tile_adds_tile(self):
        """
        Test that add_random_tile adds a tile to an empty board.
        """
        Board.empty_cells.clear()
        game = Game2048(board=Board())
        initial_state = game.board.get_state()

        game.add_random_tile()

        # Assert that the board state has changed
        assert game.board.get_state() != initial_state

        # Assert that exactly one tile has been added
        assert len(Board.get_empty_tiles(game.board.get_state())) == 15

    def test_add_random_tile_correct_values(self):
        """
        Test that add_random_tile adds only values 1 or 2 (representing 2 or 4 in the game).
        """
        game = Game2048(board=Board())

        # Run add_random_tile multiple times
        for _ in range(100):
            game.add_random_tile()

        # Check that all non-empty tiles have values 1 or 2
        state = game.board.get_state(unpack=True)
        non_empty_tiles = [tile for tile in state if tile != 0]
        assert all(tile in [1, 2] for tile in non_empty_tiles)

    def test_add_random_tile_full_board(self):
        """
        Test that add_random_tile does not modify a full board.
        """
        # Create a full board state
        full_board_state = 0xFFFFFFFFFFFFFFFF
        game = Game2048(board=Board())
        game.board.set_state(full_board_state)

        # Call add_random_tile
        game.add_random_tile()

        # Assert that the board state hasn't changed
        assert game.board.get_state() == full_board_state

    def test_add_random_tile_invalid_board_state(self):
        """
        Test that add_random_tile raises an exception for an invalid board state.
        """
        game = Game2048(board=Board())
        with pytest.raises(ValueError):
            game.board.set_state(-1)  # Invalid state
            game.add_random_tile()

    def test_add_random_tile_probability(self):
        """
        Test that add_random_tile adds 1 (representing 2 in the game) with ~90% probability.
        """
        game = Game2048(board=Board())
        ones_count = 0
        total_trials = 1000

        for _ in range(total_trials):
            game.board.set_state(0)  # Reset the board
            game.add_random_tile()
            if game.board.get_state(unpack=True)[0] == 1:
                ones_count += 1

        # Check that the probability is roughly 90% (allowing for some variance)
        assert 0.85 <= ones_count / total_trials <= 0.95

    def test_add_random_tile_probability_2(self):
        """
        Test that add_random_tile adds tiles with the correct probability (90% for 1, 10% for 2).
        """
        board = Board()
        game = Game2048(board=board)

        # Set a fixed seed for reproducibility
        random.seed(42)

        num_trials = 1000
        count_1 = 0
        count_2 = 0

        for _ in range(num_trials):
            game.board.set_state(0)  # Reset to empty board
            game.add_random_tile()
            new_state = game.board.get_state()
            added_value = new_state & 0xF  # Get the value of the first tile
            if added_value == 1:
                count_1 += 1
            elif added_value == 2:
                count_2 += 1

        # Check if the probabilities are roughly correct (allowing for some variation)
        assert 0.85 <= count_1 / num_trials <= 0.95, f"Probability of adding 1 should be around 0.9, but got {count_1 / num_trials}"
        assert 0.05 <= count_2 / num_trials <= 0.15, f"Probability of adding 2 should be around 0.1, but got {count_2 / num_trials}"

    def test_add_random_tile_when_board_has_empty_tiles(self):
        """
        Test that add_random_tile adds a tile to an empty spot on the board.
        """
        # Create a board with some empty tiles
        initial_board_state = 0x1234567800000000  # Last row is empty
        board = Board()
        board.set_state(initial_board_state)
        game = Game2048(board=board)

        # Set a fixed seed for reproducibility
        random.seed(42)

        # Call add_random_tile
        game.add_random_tile()

        # Get the new state
        new_state = game.board.get_state()

        # Verify that the new state is different from the initial state
        assert new_state != initial_board_state, "Board state should change after adding a random tile"

        # Verify that only one tile was added
        initial_empty_tiles = Board.get_empty_tiles(initial_board_state)
        new_empty_tiles = Board.get_empty_tiles(new_state)
        assert len(initial_empty_tiles) - len(new_empty_tiles) == 1, "Exactly one tile should be added"

        # Verify that the added tile is either 1 or 2 (2^1 or 2^2 in the game)
        added_tile_value = None
        for i in range(16):
            initial_value = (initial_board_state >> (i * 4)) & 0xF
            new_value = (new_state >> (i * 4)) & 0xF
            if initial_value == 0 and new_value != 0:
                added_tile_value = new_value
                break

        assert added_tile_value in [1, 2], f"Added tile value should be 1 or 2, but got {added_tile_value}"

    def test_add_random_tile_when_board_is_full(self):
        """
        Test that add_random_tile does not modify the board when it's full.
        """
        # Create a full board state
        full_board_state = 0xFFFFFFFFFFFFFFFF  # All tiles filled with non-zero values
        board = Board()
        board.set_state(full_board_state)
        game = Game2048(board=board)

        # Save the initial state
        initial_state = game.board.get_state()

        # Call add_random_tile
        game.add_random_tile()

        # Verify that the board state hasn't changed
        assert game.board.get_state() == initial_state, "Board state should not change when full"

    def test_get_score_board_state_not_initialized(self):
        """
        Test get_score when the board state is not initialized
        """
        game = Game2048()
        game.board = None
        with pytest.raises(AttributeError):
            game.get_score()

    def test_get_score_board_state_overflow(self):
        """
        Test get_score when the board state causes integer overflow
        """
        overflow_board = Board()
        overflow_board.set_state(0xFFFFFFFFFFFFFFFF)  # Maximum possible 64-bit integer
        game = Game2048(board=overflow_board)
        with pytest.raises(OverflowError):
            game.get_score()

    def test_get_score_empty_board(self):
        """
        Test get_score when the board is empty (all tiles are 0)
        """
        game = Game2048(board=Board())
        assert game.get_score() == 0

    def test_get_score_incorrect_board_type(self):
        """
        Test get_score when the board is of incorrect type
        """
        with pytest.raises(AttributeError):
            Game2048(board="not a board object").get_score()

    def test_get_score_invalid_board_state(self):
        """
        Test get_score when the board state is invalid (contains negative values)
        """
        invalid_board = Board()
        invalid_board.set_state(0b1111111111111111111111111111111111111111111111111111111111111111)  # All tiles set to 15 (invalid state)
        game = Game2048(board=invalid_board)
        with pytest.raises(ValueError):
            game.get_score()

    def test_get_score_returns_correct_sum(self):
        """
        Test that get_score returns the correct sum of 2^tile for all non-zero tiles on the board.
        """
        # Create a board with known state
        board = Board()
        board.set_state(0b0001_0010_0011_0100)  # 1, 2, 3, 4 in binary

        # Create a game with the known board
        game = Game2048(board=board)

        # Calculate expected score
        expected_score = 2**1 + 2**2 + 2**3 + 2**4

        # Assert that get_score returns the expected score
        assert game.get_score() == expected_score

    def test_init_with_all_parameters(self):
        """
        Test initialization with all parameters provided.
        """
        board = Board()
        player = RandomPlayer()
        move_count = 10
        interface = object()

        game = Game2048(board=board, player=player, move_count=move_count, interface=interface)

        assert game.board == board
        assert game.player == player
        assert game.move_count == move_count
        assert game.interface == interface

    def test_init_with_custom_board_and_player(self):
        """
        Test initializing Game2048 with a custom board and player.
        """
        # Create a custom board with less than 14 empty tiles
        custom_board = Board()
        for _ in range(3):  # Fill some tiles to ensure less than 14 empty tiles
            custom_board.set_state(Board.set_tile(custom_board.get_state(), 0, 0, 2))
            custom_board.set_state(Board.set_tile(custom_board.get_state(), 1, 1, 2))
            custom_board.set_state(Board.set_tile(custom_board.get_state(), 2, 2, 2))

        # Create a custom player
        class CustomPlayer(Player):
            def choose_action(self, valid_actions):
                return valid_actions[0]

        custom_player = CustomPlayer()

        # Initialize the game with custom board and player
        game = Game2048(board=custom_board, player=custom_player)

        # Assert that the game's board is the custom board
        assert game.board.get_state() == custom_board.get_state()

        # Assert that the game's player is the custom player
        assert isinstance(game.player, CustomPlayer)

        # Assert that the number of empty tiles is less than or equal to 14
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert len(empty_tiles) <= 14

        # Assert that the move count is initialized to 0
        assert game.move_count == 0

        # Assert that the interface is None
        assert game.interface is None

    def test_init_with_custom_board_and_player_2(self):
        """
        Test initialization of Game2048 with a custom board and player.
        """
        custom_board = Board()
        custom_player = RandomPlayer()
        game = Game2048(board=custom_board, player=custom_player)

        assert game.board == custom_board
        assert game.player == custom_player
        assert game.move_count == 0
        assert game.interface is None

        # Check that random tiles were added
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert len(empty_tiles) <= 14

    def test_init_with_custom_interface(self):
        """
        Test initialization with a custom interface object.
        """
        class DummyInterface:
            pass

        interface = DummyInterface()
        game = Game2048(interface=interface)
        assert game.interface == interface

    def test_init_with_custom_player_and_empty_board(self):
        """
        Test initialization of Game2048 with a custom player and an empty board.
        """
        custom_player = RandomPlayer()
        game = Game2048(player=custom_player)

        assert isinstance(game.board, Board)
        assert game.player == custom_player
        assert game.move_count == 0
        assert game.interface is None

        # Check that random tiles were added
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert len(empty_tiles) <= 14

        # Verify that the board is not completely empty
        assert len(empty_tiles) < 16

    def test_init_with_default_board_and_custom_player(self):
        """
        Test initialization of Game2048 with default board and custom player.
        """
        # Arrange
        custom_player = RandomPlayer()

        # Act
        game = Game2048(player=custom_player)

        # Assert
        assert isinstance(game.board, Board)
        assert game.player == custom_player
        assert game.move_count == 0
        assert game.interface is None

        # Check that the board is initialized with 2 to 4 tiles
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert 12 <= len(empty_tiles) <= 14

    def test_init_with_default_board_and_random_player(self):
        """
        Test initialization of Game2048 with default board and random player.
        """
        game = Game2048()

        assert isinstance(game.board, Board)
        assert isinstance(game.player, RandomPlayer)
        assert game.move_count == 0
        assert game.interface is None

        # Check that the board has been initialized with some tiles
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert len(empty_tiles) <= 14

    def test_init_with_default_values(self):
        """
        Test initialization of Game2048 with default values.
        """
        game = Game2048()

        assert isinstance(game.board, Board)
        assert isinstance(game.player, RandomPlayer)
        assert game.move_count == 0
        assert game.interface is None

        # Check that random tiles were added
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert len(empty_tiles) <= 14

    def test_init_with_full_board(self):
        """
        Test initialization with a full board (no empty tiles).
        """
        full_board = Board()
        full_board.set_state(0xFFFFFFFFFFFFFFFF)  # All tiles filled
        game = Game2048(board=full_board)
        assert len(Board.get_empty_tiles(game.board.get_state())) == 0

    def test_init_with_invalid_board(self):
        """
        Test initialization with an invalid board object.
        """
        with pytest.raises(AttributeError):
            Game2048(board="invalid_board")

    def test_init_with_invalid_player(self):
        """
        Test initialization with an invalid player object.
        """
        with pytest.raises(AttributeError):
            Game2048(player="invalid_player")

    def test_init_with_negative_move_count(self):
        """
        Test initialization with a negative move count.
        """
        with pytest.raises(ValueError):
            Game2048(move_count=-1)

    def test_init_with_non_integer_move_count(self):
        """
        Test initialization with a non-integer move count.
        """
        with pytest.raises(TypeError):
            Game2048(move_count="10")

    def test_play_game_exception_handling(self):
        """
        Test play_game exception handling
        """
        class BrokenPlayer(RandomPlayer):
            def choose_action(self, valid_actions):
                raise Exception("Simulated error")

        game = Game2048(player=BrokenPlayer())
        with pytest.raises(Exception, match="Simulated error"):
            game.play_game()

    def test_play_game_no_valid_moves(self):
        """
        Test play_game when there are no valid moves available.
        """
        # Create a mock board with no valid moves
        mock_board = MagicMock(spec=Board)
        mock_board.get_state.return_value = 0xFFFFFFFFFFFFFFFF  # Full board state

        # Create a mock player
        mock_player = MagicMock(spec=Player)

        # Create the game with mocked dependencies
        game = Game2048(board=mock_board, player=mock_player)

        # Mock the play_move method to return False (no valid moves)
        game.play_move = MagicMock(return_value=False)

        # Set up expectations
        expected_score = 65536  # Example score for a full board
        expected_state = 0xFFFFFFFFFFFFFFFF
        expected_move_count = 0

        # Call the method under test
        score, state, move_count = game.play_game()

        # Assertions
        assert score == expected_score
        assert state == expected_state
        assert move_count == expected_move_count
        game.play_move.assert_called_once()
        mock_board.get_state.assert_called()

    def test_play_game_returns_correct_output(self):
        """
        Test that play_game method returns the correct output format and types.
        """
        game = Game2048(player=RandomPlayer(), interface=CLI2048())
        result = game.play_game()

        assert isinstance(result, tuple)
        assert len(result) == 3

        score, state, move_count = result

        assert isinstance(score, int)
        assert isinstance(state, int)
        assert isinstance(move_count, int)

        assert score >= 0
        assert state > 0
        assert move_count >= 0

        # Check if the final state is valid
        board = Board(state)
        assert len(Board.get_empty_tiles(board.get_state())) == 0

        # Verify that the score matches the state
        calculated_score = sum([2 ** tile for tile in board.get_state(unpack=True) if tile > 0])
        assert score == calculated_score

    def test_play_game_with_full_board(self):
        """
        Test play_game with a full board (no valid moves)
        """
        full_board = Board()
        full_board.set_state(0x1111111111111111)  # All tiles filled with 1
        game = Game2048(board=full_board, player=RandomPlayer())
        score, state, move_count = game.play_game()
        assert score == 32
        assert state == 0x1111111111111111
        assert move_count == 0

    def test_play_game_with_interface(self):
        """
        Test play_game method with an interface and ensure it returns correct values.
        """
        # Initialize the game with a mock interface
        mock_interface = CLI2048()
        game = Game2048(player=RandomPlayer(), interface=mock_interface)

        # Play the game
        score, final_state, move_count = game.play_game()

        # Assert that the returned values are of the correct type
        assert isinstance(score, int)
        assert isinstance(final_state, int)
        assert isinstance(move_count, int)

        # Assert that the game has actually been played
        assert move_count > 0

        # Assert that the final state is valid
        assert 0 <= final_state < (1 << 64)  # 64-bit integer

        # Assert that the score is consistent with the final state
        unpacked_state = Board.get_unpacked_state(final_state)
        calculated_score = sum(2 ** tile for tile in unpacked_state if tile > 0)
        assert score == calculated_score

        # Assert that the interface was updated at least once
        # This is a simplistic check and might need to be adjusted based on how update() is implemented
        assert hasattr(mock_interface, 'update')

    def test_play_game_with_invalid_board(self):
        """
        Test play_game with an invalid board state
        """
        with pytest.raises(ValueError):
            invalid_board = Board()
            invalid_board.set_state(0xFFFFFFFFFFFFFFFF)  # All tiles filled with max value
            game = Game2048(board=invalid_board)
            game.play_game()

    def test_play_game_with_invalid_player(self):
        """
        Test play_game with an invalid player type
        """
        with pytest.raises(AttributeError):
            game = Game2048(player="InvalidPlayer")
            game.play_game()

    def test_play_game_with_none_interface(self):
        """
        Test play_game with None interface
        """
        game = Game2048(player=RandomPlayer(), interface=None)
        score, state, move_count = game.play_game()
        assert isinstance(score, int)
        assert isinstance(state, int)
        assert isinstance(move_count, int)

    def test_play_move_2(self):
        """
        Test play_move when there are no valid actions available.
        Expected behavior: The method should return False.
        """
        # Create a mock player
        class MockPlayer(Player):
            def choose_action(self, valid_actions):
                return None  # This should never be called in this test

        # Create a board with no valid moves
        full_board_state = 0xFFFFFFFFFFFFFFFF  # All cells filled with 2^15
        board = Board()
        board.set_state(full_board_state)

        # Create a game with the full board and mock player
        game = Game2048(board=board, player=MockPlayer(), interface=CLI2048())

        # Call play_move and assert that it returns False
        assert game.play_move() == False

        # Ensure that the board state hasn't changed
        assert game.board.get_state() == full_board_state

    def test_play_move_board_full_after_move(self):
        """
        Test play_move when the board becomes full after making a move.
        """
        board = Board()
        board.set_state(0xFFFFFFFFFFFFFFF0)  # Almost full board state
        player = Player()
        game = Game2048(board=board, player=player)

        # Mock player's choose_action to return a valid action
        def mock_choose_action(valid_actions):
            return valid_actions[0]

        player.choose_action = mock_choose_action

        result = game.play_move()

        assert result == True
        assert game.board.get_state() == 0xFFFFFFFFFFFFFFFF

    def test_play_move_exception_in_choose_action(self):
        """
        Test play_move when an exception occurs in the player's choose_action method.
        """
        board = Board()
        player = Player()
        game = Game2048(board=board, player=player)

        # Mock player's choose_action to raise an exception
        def mock_choose_action(valid_actions):
            raise ValueError("Test exception")

        player.choose_action = mock_choose_action

        with pytest.raises(ValueError, match="Test exception"):
            game.play_move()

    def test_play_move_invalid_next_state(self):
        """
        Test play_move when the player returns an invalid next state.
        """
        board = Board()
        player = Player()
        game = Game2048(board=board, player=player)

        # Mock player's choose_action to return an invalid next state
        def mock_choose_action(valid_actions):
            return (valid_actions[0][0], 0xFFFFFFFFFFFFFFFF)  # Invalid next state

        player.choose_action = mock_choose_action

        with pytest.raises(ValueError):
            game.play_move()

    def test_play_move_no_valid_actions(self):
        """
        Test play_move when there are no valid actions available.
        """
        board = Board()
        board.set_state(0xFFFFFFFFFFFFFFFF)  # Full board state
        player = Player()
        game = Game2048(board=board, player=player)

        result = game.play_move()

        assert result == False

    def test_play_move_player_returns_invalid_action(self):
        """
        Test play_move when the player returns an invalid action.
        """
        board = Board()
        player = Player()
        game = Game2048(board=board, player=player)

        # Mock player's choose_action to return an invalid action
        def mock_choose_action(valid_actions):
            return (None, 0)  # Invalid action

        player.choose_action = mock_choose_action

        with pytest.raises(TypeError):
            game.play_move()

    def test_play_move_valid_action(self):
        """
        Test play_move when there are valid actions available.
        Expect the method to return True and update the game state.
        """
        # Initialize the game with a known board state
        initial_state = 0x1111000000000000  # Board with 4 tiles of value 2 in the top row
        board = Board()
        board.set_state(initial_state)
        player = RandomPlayer()
        game = Game2048(board=board, player=player)

        # Ensure there are valid moves
        assert Board.get_valid_move_actions(board.get_state())

        # Call the method under test
        result = game.play_move()

        # Assert that the method returns True
        assert result == True

        # Assert that the board state has changed
        assert game.board.get_state() != initial_state

        # Assert that a new tile has been added (15 non-empty tiles instead of 14)
        assert len(Board.get_empty_tiles(game.board.get_state())) == 15

    def test_reset_after_game_over(self):
        """
        Test reset method after the game is over (no valid moves).
        """
        game = Game2048()
        game.board.set_state(0x1111111111111111)  # Set all tiles to 2, no valid moves
        game.reset()
        assert game.play_move()

    def test_reset_board_state(self):
        """
        Test that the board state is correctly reset.
        """
        game = Game2048()
        game.board.set_state(0xFFFFFFFFFFFFFFFF)  # Set all tiles to non-zero
        game.reset()
        assert sum(game.board.get_state(unpack=True)) == 2

    def test_reset_game_state(self):
        """
        Test that the reset method properly initializes the game state.
        """
        # Initialize the game with a non-empty board and non-zero move count
        initial_board = Board()
        initial_board.set_state(0x1234567890ABCDEF)  # Some arbitrary non-empty state
        initial_move_count = 10
        game = Game2048(board=initial_board, move_count=initial_move_count, player=RandomPlayer())

        # Call the reset method
        game.reset()

        # Check that the board is reset to an empty state except for two random tiles
        assert game.board.get_state() != 0  # Board should not be completely empty
        assert bin(game.board.get_state()).count('1') == 2  # Two tiles should be set

        # Check that the move count is reset to 0
        assert game.move_count == 0

        # Check that exactly two random tiles were added
        empty_tiles = Board.get_empty_tiles(game.board.get_state())
        assert len(empty_tiles) == 14

    def test_reset_with_custom_move_count(self):
        """
        Test reset method when the game is initialized with a custom move count.
        """
        game = Game2048(move_count=100)
        game.reset()
        assert game.move_count == 0

    def test_reset_with_custom_player(self):
        """
        Test reset method when the game is initialized with a custom player.
        """
        custom_player = RandomPlayer()
        game = Game2048(player=custom_player)
        original_player = game.player
        game.reset()
        assert game.player == original_player

    def test_reset_with_invalid_board(self):
        """
        Test reset method when the game is initialized with an invalid board.
        """
        with pytest.raises(ValueError):
            game = Game2048(board=Board(state=2**64))  # Invalid state
            game.reset()

# Create a test_data.py file to store test constants
TEST_BOARD_STATES = {
    'full': 0xFFFFFFFFFFFFFFFF,
    'empty': 0x0000000000000000,
    'partial': 0x1234567890ABCDEF
}

def create_test_game(board_state=None, player=None):
    board = Board()
    if board_state:
        board.set_state(board_state)
    return Game2048(board=board, player=player or RandomPlayer())
