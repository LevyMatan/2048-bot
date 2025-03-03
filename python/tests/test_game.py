import unittest
from src.game2048.game import Game2048
from src.game2048.board import Board
from src.game2048.players import RandomPlayer
from src.game2048.interfaces import GYM2048

class TestGame2048(unittest.TestCase):
    def setUp(self):
        """Set up test fixtures before each test method."""
        interface = GYM2048()
        interface.set_total_games(1)  # Set total games to 1 for testing
        self.game = Game2048(player=RandomPlayer(), interface=interface)

    def test_initialization(self):
        """Test game initialization."""
        # Test default initialization
        game = Game2048()
        self.assertIsNotNone(game.board)
        self.assertEqual(game.move_count, 0)
        self.assertEqual(game.score, 0)
        self.assertIsInstance(game.player, RandomPlayer)

        # Test custom initialization
        custom_board = Board()
        custom_player = RandomPlayer()
        game = Game2048(board=custom_board, player=custom_player, move_count=5)
        self.assertEqual(game.board, custom_board)
        self.assertEqual(game.player, custom_player)
        self.assertEqual(game.move_count, 5)

    def test_add_random_tile(self):
        """Test adding random tiles to the board."""
        self.game.reset()  # Start with empty board
        initial_empty_tiles = len(Board.get_empty_tiles(self.game.board.get_state()))
        self.game.add_random_tile()
        new_empty_tiles = len(Board.get_empty_tiles(self.game.board.get_state()))
        self.assertEqual(initial_empty_tiles - 1, new_empty_tiles)

    def test_reset(self):
        """Test game reset functionality."""
        # Play some moves to change the game state
        self.game.play_move()
        self.game.move_count = 10
        self.game.score = 100

        # Reset the game
        self.game.reset()
        
        # Verify reset state
        self.assertEqual(self.game.move_count, 0)
        self.assertEqual(self.game.score, 0)
        empty_tiles = len(Board.get_empty_tiles(self.game.board.get_state()))
        self.assertEqual(empty_tiles, 14)  # Should have 2 tiles placed after reset

    def test_play_move(self):
        """Test playing a single move."""
        self.game.reset()
        initial_state = self.game.board.get_state()
        move_result = self.game.play_move()
        
        # Verify that the move was made
        self.assertTrue(move_result)
        current_state = self.game.board.get_state()
        self.assertNotEqual(initial_state, current_state)

    def test_play_game(self):
        """Test playing a complete game."""
        score, final_state, move_count = self.game.play_game()
        
        # Verify game completion
        self.assertIsNotNone(score)
        self.assertIsNotNone(final_state)
        self.assertGreaterEqual(move_count, 0)
        
        # Verify game is in terminal state
        valid_actions = Board.get_valid_move_actions(final_state)
        self.assertEqual(len(valid_actions), 0)

    def test_get_score(self):
        """Test score tracking."""
        self.game.reset()
        initial_score = self.game.get_score()
        self.assertEqual(initial_score, 0)
        
        # Play a move and verify score changes
        self.game.play_move()
        new_score = self.game.get_score()
        self.assertGreaterEqual(new_score, initial_score)

if __name__ == '__main__':
    unittest.main() 