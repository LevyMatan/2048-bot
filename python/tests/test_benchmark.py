import unittest
from unittest.mock import patch, MagicMock
from src.game2048.benchmark import get_highest_tile, run_benchmark, generate_report
from src.game2048.board import Board
from src.game2048.players import RandomPlayer, MaxEmptyCellsPlayer
from src.game2048.game import Game2048
from collections import defaultdict
import time

class TestBenchmark(unittest.TestCase):
    def setUp(self):
        """Set up test fixtures before each test method."""
        if not Board.is_lookup_tables_initialized():
            Board._Board__init_lookup_tables()

    def test_get_highest_tile(self):
        """Test getting the highest tile value from different board states."""
        # Empty board
        self.assertEqual(get_highest_tile(0x0000_0000_0000_0000), 0)
        
        # Board with single tiles
        self.assertEqual(get_highest_tile(0x1000_0000_0000_0000), 2)  # 2^1
        self.assertEqual(get_highest_tile(0x2000_0000_0000_0000), 4)  # 2^2
        
        # Board with multiple tiles
        self.assertEqual(get_highest_tile(0x1234_0000_0000_0000), 16)  # 2^4
        self.assertEqual(get_highest_tile(0xFFFF_0000_0000_0000), 32768)  # 2^15

    @patch('time.time')
    @patch('src.game2048.game.Game2048.play_game')
    def test_run_benchmark_basic(self, mock_play_game, mock_time):
        """Test basic benchmark functionality with mocked game play."""
        # Mock time to return increasing values
        mock_time.side_effect = [0, 1.5]  # Start time and end time
        
        # Mock game results
        mock_play_game.return_value = (100, 0x1234_0000_0000_0000, 50)
        
        # Run benchmark with minimal settings
        results = run_benchmark(
            num_games=2,
            players=[RandomPlayer],
            optimize=False,
            show_progress=False
        )
        
        # Verify results structure
        self.assertIn('Random', results)
        player_results = results['Random']
        
        # Check metrics
        self.assertEqual(player_results['avg_score'], 100)
        self.assertEqual(player_results['max_score'], 100)
        self.assertEqual(player_results['avg_moves'], 50)
        self.assertEqual(player_results['time_per_game'], 0.75)  # 1.5 seconds / 2 games
        
        # Verify number of games played
        self.assertEqual(mock_play_game.call_count, 2)

    @patch('time.time')
    @patch('src.game2048.game.Game2048.play_game')
    def test_run_benchmark_multiple_players(self, mock_play_game, mock_time):
        """Test benchmark with multiple players."""
        # Mock time to return increasing values for each player
        mock_time.side_effect = [0, 2.0, 2.0, 4.0]  # Start and end times for each player
        
        # Mock different results for different games
        mock_play_game.side_effect = [
            (100, 0x1234_0000_0000_0000, 50),  # First player game 1
            (200, 0x2234_0000_0000_0000, 60),  # First player game 2
            (300, 0x3234_0000_0000_0000, 70),  # Second player game 1
            (400, 0x4234_0000_0000_0000, 80),  # Second player game 2
        ]
        
        # Run benchmark with two players
        results = run_benchmark(
            num_games=2,
            players=[RandomPlayer, MaxEmptyCellsPlayer],
            optimize=False,
            show_progress=False
        )
        
        # Verify results for both players
        self.assertIn('Random', results)
        self.assertIn('MaxEmptyCells', results)
        
        # Check first player metrics
        random_results = results['Random']
        self.assertEqual(random_results['avg_score'], 150)  # (100 + 200) / 2
        self.assertEqual(random_results['max_score'], 200)
        self.assertEqual(random_results['avg_moves'], 55)  # (50 + 60) / 2
        self.assertEqual(random_results['time_per_game'], 1.0)  # 2.0 seconds / 2 games
        
        # Check second player metrics
        max_empty_results = results['MaxEmptyCells']
        self.assertEqual(max_empty_results['avg_score'], 350)  # (300 + 400) / 2
        self.assertEqual(max_empty_results['max_score'], 400)
        self.assertEqual(max_empty_results['avg_moves'], 75)  # (70 + 80) / 2
        self.assertEqual(max_empty_results['time_per_game'], 1.0)  # 2.0 seconds / 2 games

    @patch('src.game2048.interfaces.CLI2048.pretty_print')
    def test_generate_report(self, mock_pretty_print):
        """Test report generation with mock benchmark results."""
        # Create mock benchmark results
        mock_results = {
            'Random': {
                'avg_score': 1000.5,
                'max_score': 2048,
                'best_state': 0x1234_0000_0000_0000,
                'best_moves': 100,
                'avg_moves': 150.5,
                'time_per_game': 0.1,
                'total_time': 10.0,
                'highest_tile_counts': defaultdict(int, {
                    2: 10,
                    4: 20,
                    8: 30,
                    16: 40
                })
            }
        }
        
        # Generate report
        report = generate_report(mock_results)
        
        # Verify report content
        self.assertIsInstance(report, str)
        self.assertIn('BENCHMARK RESULTS', report)
        self.assertIn('Random', report)
        self.assertIn('1000.5', report)  # avg_score
        self.assertIn('2048', report)    # max_score
        self.assertIn('150.5', report)   # avg_moves
        
        # Verify pretty_print was called
        mock_pretty_print.assert_called_once_with(
            0x1234_0000_0000_0000,  # state
            2048,  # score
            100    # moves
        )

    @patch('src.game2048.game.Game2048.play_game')
    def test_benchmark_with_optimization(self, mock_play_game):
        """Test that the optimize flag properly handles board optimization."""
        mock_play_game.return_value = (100, 0x1234_0000_0000_0000, 50)
        
        # Run benchmark with optimization enabled
        run_benchmark(num_games=1, players=[RandomPlayer], optimize=True, show_progress=False)
        
        # Run benchmark without optimization
        run_benchmark(num_games=1, players=[RandomPlayer], optimize=False, show_progress=False)
        
        # Both runs should complete without errors
        self.assertEqual(mock_play_game.call_count, 2)

if __name__ == '__main__':
    unittest.main() 