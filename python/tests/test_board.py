import pytest
from game2048.board import Board, Action

class TestBoard:
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
        with pytest.raises((TypeError, ValueError)):
            Board("invalid string")
            
        with pytest.raises(ValueError):
            Board(-1)
            
        with pytest.raises(OverflowError):
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
            assert state == expected_state, f"Failed for input {input_row:04x}"
            assert score == expected_score, f"Wrong score for input {input_row:04x}"

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
            assert state == expected_state, f"Failed for input {input_row:04x}"
            assert score == expected_score, f"Wrong score for input {input_row:04x}"

    def test_simulate_moves(self):
        """Test simulation of all possible moves"""
        # Board state:
        # 1 1 0 0
        # 0 0 0 0
        # 0 0 0 0
        # 0 0 0 0
        initial_state = 0x1100_0000_0000_0000
        
        moves = Board.simulate_moves(initial_state)
        assert len(moves) == 4  # Should return 4 moves
        
        left_state, left_score = moves[0]  # LEFT
        right_state, right_score = moves[1]  # RIGHT
        up_state, up_score = moves[2]  # UP
        down_state, down_score = moves[3]  # DOWN
        
        assert left_state == 0x2000_0000_0000_0000  # Should merge to 2
        assert left_score == 4  # Score is 2^2 = 4 for merging two 1s
        
        assert right_state == 0x0002_0000_0000_0000  # Should merge to 2
        assert right_score == 4  # Score is 2^2 = 4 for merging two 1s
        
        assert up_state == initial_state  # No valid up move
        assert up_score == 0
        
        assert down_state == 0x0000_0000_0000_1100  # Should move to bottom
        assert down_score == 0

    def test_get_valid_move_actions(self):
        """Test getting valid moves"""
        # Board state:
        # 1 1 0 0
        # 0 0 0 0
        # 0 0 0 0
        # 0 0 0 0
        state = 0x1100_0000_0000_0000
        
        valid_moves = Board.get_valid_move_actions(state)
        assert len(valid_moves) == 3  # Should have 3 valid moves
        
        expected_moves = {
            (Action.LEFT, 0x2000_0000_0000_0000, 4),   # Merge left, score = 2^2
            (Action.RIGHT, 0x0002_0000_0000_0000, 4),  # Merge right, score = 2^2
            (Action.DOWN, 0x0000_0000_0000_1100, 0),   # Move down, no merge
        }
        
        assert set(valid_moves) == expected_moves

    def test_get_empty_cells(self):
        """Test getting empty cell positions"""
        # Board state:
        # 1 0 1 0
        # 0 0 0 0
        # 0 1 0 0
        # 0 0 0 1
        state = 0x1010_0000_0100_0001
        
        empty_cells = Board.get_empty_tiles(state)
        assert len(empty_cells) == 12  # Should have 12 empty cells
        
        expected_empty_cells = {
            (0, 1), (0, 3),  # Top row
            (1, 0), (1, 1), (1, 2), (1, 3),  # Second row
            (2, 0), (2, 2), (2, 3),  # Third row
            (3, 0), (3, 1), (3, 2),  # Bottom row
        }
        
        assert set(empty_cells) == expected_empty_cells

    def test_set_tile(self):
        """Test setting tile values"""
        state = 0x0000_0000_0000_0000
        
        # Set some tiles
        state = Board.set_tile(state, 0, 0, 1)  # Set top-left to 1
        assert state == 0x1000_0000_0000_0000
        
        state = Board.set_tile(state, 3, 3, 2)  # Set bottom-right to 2
        assert state == 0x1000_0000_0000_0002
        
        # Test invalid positions
        with pytest.raises(ValueError):
            Board.set_tile(state, 4, 0, 1)  # Invalid row
        
        with pytest.raises(ValueError):
            Board.set_tile(state, 0, 4, 1)  # Invalid column
            
        # Test invalid values
        with pytest.raises(ValueError):
            Board.set_tile(state, 0, 1, 16)  # Value too large
            
        with pytest.raises(ValueError):
            Board.set_tile(state, 0, 1, -1)  # Negative value
            
        # Test setting occupied tile
        with pytest.raises(ValueError):
            Board.set_tile(state, 0, 0, 2)  # Already occupied

    def test_edge_cases(self):
        """Test edge cases and special scenarios"""
        # Test merging maximum values (15)
        max_row = 0xFFFF  # [15,15,15,15]
        state, score = Board._row_left(max_row)
        assert state == 0xFFFF  # Should not merge
        assert score == 0
        
        # Test alternating values
        alternating = 0x1212  # [1,2,1,2]
        state, score = Board._row_left(alternating)
        assert state == alternating  # Should not merge
        assert score == 0
        
        # Test single value in different positions
        assert Board._row_left(0x0001)[0] == 0x1000  # [0,0,0,1] -> [1,0,0,0]
        assert Board._row_right(0x1000)[0] == 0x0001  # [1,0,0,0] -> [0,0,0,1]
