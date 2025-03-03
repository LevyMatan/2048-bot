import unittest
from src.game2048.board import Board, Action

class TestBoard(unittest.TestCase):
    def setUp(self):
        """Ensure lookup tables are initialized before each test"""
        if not Board.is_lookup_tables_initialized():
            Board._Board__init_lookup_tables()

    def test_initialization(self):
        """Test basic board initialization"""
        board = Board()
        assert board._Board__state == 0
        
        board = Board(0x1234)
        assert board._Board__state == 0x1234

    def test_initialization_validation(self):
        """Test board initialization with invalid inputs"""
        with self.assertRaises(TypeError):
            Board("invalid string")
            
        with self.assertRaises(ValueError):
            Board(-1)
            
        with self.assertRaises(OverflowError):
            Board(2**64)

    def test_row_left_basic(self):
        """Test basic left moves"""
        test_cases = [
            (0x0000, 0x0000, 0),  # [0,0,0,0] -> [0,0,0,0]
            (0x1000, 0x1000, 0),  # [1,0,0,0] -> [1,0,0,0]
            (0x1100, 0x2000, 4),  # [1,1,0,0] -> [2,0,0,0], score = 2^2 = 4
            (0x1234, 0x1234, 0),  # [1,2,3,4] -> [1,2,3,4]
            (0x1111, 0x2200, 8),  # [1,1,1,1] -> [2,2,0,0], score = 2^2 + 2^2 = 8
        ]
        
        for input_row, expected_state, expected_score in test_cases:
            state, score = Board._row_left(input_row)
            self.assertEqual(state, expected_state, f"Failed for input {input_row:04x}")
            self.assertEqual(score, expected_score, f"Wrong score for input {input_row:04x}")

    def test_row_right_basic(self):
        """Test basic right moves"""
        test_cases = [
            (0x0000, 0x0000, 0),  # [0,0,0,0] -> [0,0,0,0]
            (0x1000, 0x0001, 0),  # [1,0,0,0] -> [0,0,0,1]
            (0x0011, 0x0002, 4),  # [0,0,1,1] -> [0,0,0,2], score = 2^2 = 4
            (0x1234, 0x1234, 0),  # [1,2,3,4] -> [1,2,3,4]
            (0x1111, 0x0022, 8),  # [1,1,1,1] -> [0,0,2,2], score = 2^2 + 2^2 = 8
        ]
        
        for input_row, expected_state, expected_score in test_cases:
            state, score = Board._row_right(input_row)
            self.assertEqual(state, expected_state, f"Failed for input {input_row:04x}")
            self.assertEqual(score, expected_score, f"Wrong score for input {input_row:04x}")

    def test_simulate_moves(self):
        """Test simulation of all possible moves"""
        # Board state:
        # 1 1 0 0
        # 0 0 0 0
        # 0 0 0 0
        # 0 0 0 0
        initial_state = 0x1100_0000_0000_0000
        
        moves = Board.simulate_moves(initial_state)
        self.assertEqual(len(moves), 4)  # Should return 4 moves
        
        left_state, left_score = moves[0]  # LEFT
        right_state, right_score = moves[1]  # RIGHT
        up_state, up_score = moves[2]  # UP
        down_state, down_score = moves[3]  # DOWN
        
        self.assertEqual(left_state, 0x2000_0000_0000_0000)  # Should merge to 2
        self.assertEqual(left_score, 4)  # Score is 2^2 = 4 for merging two 1s
        
        self.assertEqual(right_state, 0x0002_0000_0000_0000)  # Should merge to 2
        self.assertEqual(right_score, 4)  # Score is 2^2 = 4 for merging two 1s
        
        self.assertEqual(up_state, initial_state)  # No valid up move
        self.assertEqual(up_score, 0)
        
        self.assertEqual(down_state, 0x0000_0000_0000_1100)  # Should move to bottom
        self.assertEqual(down_score, 0)

    def test_get_valid_move_actions(self):
        """Test getting valid moves"""
        # Board state:
        # 1 1 0 0
        # 0 0 0 0
        # 0 0 0 0
        # 0 0 0 0
        state = 0x1100_0000_0000_0000
        
        valid_moves = Board.get_valid_move_actions(state)
        self.assertEqual(len(valid_moves), 3)  # Should have 3 valid moves
        
        expected_moves = {
            (Action.LEFT, 0x2000_0000_0000_0000, 4),   # Merge left, score = 2^2
            (Action.RIGHT, 0x0002_0000_0000_0000, 4),  # Merge right, score = 2^2
            (Action.DOWN, 0x0000_0000_0000_1100, 0),   # Move down, no merge
        }
        
        self.assertEqual(set(valid_moves), expected_moves)

    def test_get_empty_cells(self):
        """Test getting empty cell positions"""
        # Board state:
        # 1 0 1 0
        # 0 0 0 0
        # 0 1 0 0
        # 0 0 0 1
        state = 0x1010_0000_0100_0001
        
        empty_cells = Board.get_empty_tiles(state)
        self.assertEqual(len(empty_cells), 12)  # Should have 12 empty cells
        
        expected_empty_cells = {
            (0, 1), (0, 3),  # Top row
            (1, 0), (1, 1), (1, 2), (1, 3),  # Second row
            (2, 0), (2, 2), (2, 3),  # Third row
            (3, 0), (3, 1), (3, 2),  # Bottom row
        }
        
        self.assertEqual(set(empty_cells), expected_empty_cells)

    def test_set_tile(self):
        """Test setting tile values"""
        state = 0x0000_0000_0000_0000
        
        # Set some tiles
        state = Board.set_tile(state, 0, 0, 1)  # Set top-left to 1
        self.assertEqual(state, 0x1000_0000_0000_0000)
        
        state = Board.set_tile(state, 3, 3, 2)  # Set bottom-right to 2
        self.assertEqual(state, 0x1000_0000_0000_0002)
        
        # Test invalid positions
        with self.assertRaises(ValueError):
            Board.set_tile(state, 4, 0, 1)  # Invalid row
        
        with self.assertRaises(ValueError):
            Board.set_tile(state, 0, 4, 1)  # Invalid column
            
        # Test invalid values
        with self.assertRaises(ValueError):
            Board.set_tile(state, 0, 1, 16)  # Value too large
            
        with self.assertRaises(ValueError):
            Board.set_tile(state, 0, 1, -1)  # Negative value
            
        # Test setting occupied tile
        with self.assertRaises(ValueError):
            Board.set_tile(state, 0, 0, 2)  # Already occupied

    def test_edge_cases(self):
        """Test edge cases and special scenarios"""
        # Test merging maximum values (15)
        max_row = 0xFFFF  # [15,15,15,15]
        state, score = Board._row_left(max_row)
        self.assertEqual(state, 0xFFFF)  # Should not merge
        self.assertEqual(score, 0)
        
        # Test alternating values
        alternating = 0x1212  # [1,2,1,2]
        state, score = Board._row_left(alternating)
        self.assertEqual(state, alternating)  # Should not merge
        self.assertEqual(score, 0)
        
        # Test single value in different positions
        self.assertEqual(Board._row_left(0x0001)[0], 0x1000)  # [0,0,0,1] -> [1,0,0,0]
        self.assertEqual(Board._row_right(0x1000)[0], 0x0001)  # [1,0,0,0] -> [0,0,0,1]
