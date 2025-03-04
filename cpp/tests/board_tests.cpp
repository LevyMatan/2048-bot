#include "board.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <tuple>
#include <algorithm>
#include <random>
#include <iomanip>
#include <iostream>
#include <sstream>

class BoardTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize a new board for each test
        board = new Board();
    }

    void TearDown() override {
        delete board;
    }

    Board* board;

    // Helper function to create a board state with specific tiles
    uint64_t createBoardState(const std::vector<std::vector<int>>& tiles) {
        uint64_t state = 0;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                if (tiles[row][col] > 0) {
                    int value = Board::tileToValue(tiles[row][col]);
                    state = Board::setTile(state, row, col, value);
                }
            }
        }
        return state;
    }

    // Helper function to get tile value at a specific position
    int getTileAt(uint64_t state, int row, int col) {
        int pos = (row * 4 + col) * 4;
        int value = (state >> pos) & 0xF;
        return value == 0 ? 0 : Board::valueToTile(value);
    }

    // Helper function to print a board state for debugging
    void printBoardState(uint64_t state) {
        std::cout << "Board state:" << std::endl;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                int tile = getTileAt(state, row, col);
                std::cout << std::setw(5) << tile << " ";
            }
            std::cout << std::endl;
        }
    }
};

// Test initialization of a new board
TEST_F(BoardTest, InitializationTest) {
    EXPECT_EQ(board->getState(), 0);
}

// Test setting and getting the board state
TEST_F(BoardTest, SetAndGetStateTest) {
    uint64_t testState = 0x123456789ABCDEF0;
    board->setState(testState);
    EXPECT_EQ(board->getState(), testState);
}

// Test the tileToValue and valueToTile conversion functions
TEST_F(BoardTest, TileValueConversionTest) {
    // Test tileToValue
    EXPECT_EQ(Board::tileToValue(2), 1);
    EXPECT_EQ(Board::tileToValue(4), 2);
    EXPECT_EQ(Board::tileToValue(8), 3);
    EXPECT_EQ(Board::tileToValue(16), 4);
    EXPECT_EQ(Board::tileToValue(32), 5);
    EXPECT_EQ(Board::tileToValue(64), 6);
    EXPECT_EQ(Board::tileToValue(128), 7);
    EXPECT_EQ(Board::tileToValue(256), 8);
    EXPECT_EQ(Board::tileToValue(512), 9);
    EXPECT_EQ(Board::tileToValue(1024), 10);
    EXPECT_EQ(Board::tileToValue(2048), 11);
    EXPECT_EQ(Board::tileToValue(4096), 12);
    EXPECT_EQ(Board::tileToValue(8192), 13);
    EXPECT_EQ(Board::tileToValue(16384), 14);
    EXPECT_EQ(Board::tileToValue(32768), 15);

    // Test valueToTile
    EXPECT_EQ(Board::valueToTile(1), 2);
    EXPECT_EQ(Board::valueToTile(2), 4);
    EXPECT_EQ(Board::valueToTile(3), 8);
    EXPECT_EQ(Board::valueToTile(4), 16);
    EXPECT_EQ(Board::valueToTile(5), 32);
    EXPECT_EQ(Board::valueToTile(6), 64);
    EXPECT_EQ(Board::valueToTile(7), 128);
    EXPECT_EQ(Board::valueToTile(8), 256);
    EXPECT_EQ(Board::valueToTile(9), 512);
    EXPECT_EQ(Board::valueToTile(10), 1024);
    EXPECT_EQ(Board::valueToTile(11), 2048);
    EXPECT_EQ(Board::valueToTile(12), 4096);
    EXPECT_EQ(Board::valueToTile(13), 8192);
    EXPECT_EQ(Board::valueToTile(14), 16384);
    EXPECT_EQ(Board::valueToTile(15), 32768);
}

// Test setting a tile on the board
TEST_F(BoardTest, SetTileTest) {
    uint64_t state = 0;
    
    // Set a tile at position (0, 0) with value 2 (internal value 1)
    state = Board::setTile(state, 0, 0, 1);
    EXPECT_EQ((state >> 0) & 0xF, 1);
    
    // Set a tile at position (1, 2) with value 4 (internal value 2)
    state = Board::setTile(state, 1, 2, 2);
    EXPECT_EQ((state >> ((1 * 4 + 2) * 4)) & 0xF, 2);
    
    // Set a tile at position (3, 3) with value 8 (internal value 3)
    state = Board::setTile(state, 3, 3, 3);
    EXPECT_EQ((state >> ((3 * 4 + 3) * 4)) & 0xF, 3);
}

// Test getting empty tiles
TEST_F(BoardTest, GetEmptyTilesTest) {
    // Create a board with some tiles
    std::vector<std::vector<int>> tiles = {
        {2, 0, 0, 0},
        {0, 4, 0, 0},
        {0, 0, 8, 0},
        {0, 0, 0, 16}
    };
    uint64_t state = createBoardState(tiles);
    
    // Get empty tiles
    auto emptyTiles = Board::getEmptyTiles(state);
    
    // Check the number of empty tiles
    EXPECT_EQ(emptyTiles.size(), 12);
    
    // Check that all empty positions are included
    std::vector<std::tuple<int, int>> expectedEmptyTiles = {
        {0, 1}, {0, 2}, {0, 3},
        {1, 0}, {1, 2}, {1, 3},
        {2, 0}, {2, 1}, {2, 3},
        {3, 0}, {3, 1}, {3, 2}
    };
    
    // Sort both vectors for comparison
    std::sort(emptyTiles.begin(), emptyTiles.end());
    std::sort(expectedEmptyTiles.begin(), expectedEmptyTiles.end());
    
    EXPECT_EQ(emptyTiles, expectedEmptyTiles);
}

// Test simulating moves with scores
TEST_F(BoardTest, SimulateMovesWithScoresTest) {
    // Create a board with a simple pattern
    std::vector<std::vector<int>> tiles = {
        {2, 2, 0, 0},
        {0, 4, 4, 0},
        {0, 0, 8, 8},
        {0, 0, 0, 0}
    };
    uint64_t state = createBoardState(tiles);
    
    // Simulate moves
    auto moves = Board::simulateMovesWithScores(state);
    
    // Check that we have 4 moves
    EXPECT_EQ(moves.size(), 4);
    
    // Check LEFT move (index 0)
    uint64_t leftState = std::get<0>(moves[0]);
    int leftScore = std::get<1>(moves[0]);
    
    // Print the board state for debugging
    // printBoardState(leftState);
    
    // Verify that the board has changed after the move
    EXPECT_NE(leftState, state);
    
    // Verify that the score is positive
    EXPECT_GT(leftScore, 0);
    
    // Check RIGHT move (index 1)
    uint64_t rightState = std::get<0>(moves[1]);
    int rightScore = std::get<1>(moves[1]);
    
    // Verify that the board has changed after the move
    EXPECT_NE(rightState, state);
    
    // Verify that the score is positive
    EXPECT_GT(rightScore, 0);
}

// Test getting valid move actions
TEST_F(BoardTest, GetValidMoveActionsTest) {
    // Create a board with a pattern that allows only certain moves
    std::vector<std::vector<int>> tiles = {
        {2, 4, 8, 16},
        {32, 64, 128, 256},
        {512, 1024, 2048, 4096},
        {8192, 16384, 32768, 0}
    };
    uint64_t state = createBoardState(tiles);
    
    // Get valid move actions
    auto validMoves = Board::getValidMoveActions(state);
    
    // At least one move should be valid
    EXPECT_GT(validMoves.size(), 0);
    
    // Create a different board with more valid moves
    tiles = {
        {2, 2, 4, 8},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    state = createBoardState(tiles);
    
    // Get valid move actions
    validMoves = Board::getValidMoveActions(state);
    
    // Multiple moves should be valid
    EXPECT_GT(validMoves.size(), 1);
    
    // Check that the actions include at least LEFT and DOWN
    std::vector<Action> actions;
    for (const auto& move : validMoves) {
        actions.push_back(std::get<0>(move));
    }
    
    EXPECT_TRUE(std::find(actions.begin(), actions.end(), Action::LEFT) != actions.end() ||
                std::find(actions.begin(), actions.end(), Action::RIGHT) != actions.end());
}

// Test getting valid move actions with scores
TEST_F(BoardTest, GetValidMoveActionsWithScoresTest) {
    // Create a board with a simple pattern
    std::vector<std::vector<int>> tiles = {
        {2, 2, 0, 0},
        {0, 4, 4, 0},
        {0, 0, 8, 8},
        {0, 0, 0, 0}
    };
    uint64_t state = createBoardState(tiles);
    
    // Get valid move actions with scores
    auto validMoves = Board::getValidMoveActionsWithScores(state);
    
    // All 4 moves should be valid
    EXPECT_EQ(validMoves.size(), 4);
    
    // Check that at least one move has a positive score
    bool hasPositiveScore = false;
    for (const auto& [action, nextState, score] : validMoves) {
        if (score > 0) {
            hasPositiveScore = true;
            break;
        }
    }
    EXPECT_TRUE(hasPositiveScore);
}

// Test edge cases for move operations
TEST_F(BoardTest, MoveEdgeCasesTest) {
    // Test case 1: Full board with no possible merges
    std::vector<std::vector<int>> tiles = {
        {2, 4, 8, 16},
        {32, 64, 128, 256},
        {512, 1024, 2048, 4096},
        {8192, 16384, 32768, 2}
    };
    uint64_t state = createBoardState(tiles);
    
    // No valid moves should be available
    auto validMoves = Board::getValidMoveActions(state);
    EXPECT_EQ(validMoves.size(), 0);
    
    // Test case 2: Board with maximum value tiles (32768 = 2^15)
    tiles = {
        {32768, 32768, 32768, 32768},
        {32768, 32768, 32768, 32768},
        {32768, 32768, 32768, 32768},
        {32768, 32768, 32768, 32768}
    };
    state = createBoardState(tiles);
    
    // No valid moves should be available (max value tiles can't merge)
    validMoves = Board::getValidMoveActions(state);
    EXPECT_EQ(validMoves.size(), 0);
}

// Test random board generation and operations
TEST_F(BoardTest, RandomBoardTest) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> valueDist(1, 11); // Values from 2 to 2048
    
    // Create 10 random boards and test operations
    for (int i = 0; i < 10; ++i) {
        std::vector<std::vector<int>> tiles(4, std::vector<int>(4, 0));
        
        // Fill about 10 random positions
        for (int j = 0; j < 10; ++j) {
            int row = gen() % 4;
            int col = gen() % 4;
            if (tiles[row][col] == 0) {
                tiles[row][col] = Board::valueToTile(valueDist(gen));
            }
        }
        
        uint64_t state = createBoardState(tiles);
        
        // Test that simulateMoves and simulateMovesWithScores are consistent
        auto movesWithScores = Board::simulateMovesWithScores(state);
        auto moves = Board::simulateMoves(state);
        
        EXPECT_EQ(movesWithScores.size(), moves.size());
        for (size_t j = 0; j < moves.size(); ++j) {
            EXPECT_EQ(std::get<0>(movesWithScores[j]), moves[j]);
        }
        
        // Test that getValidMoveActions and getValidMoveActionsWithScores are consistent
        auto validMovesWithScores = Board::getValidMoveActionsWithScores(state);
        auto validMoves = Board::getValidMoveActions(state);
        
        EXPECT_EQ(validMovesWithScores.size(), validMoves.size());
        for (size_t j = 0; j < validMoves.size(); ++j) {
            EXPECT_EQ(std::get<0>(validMovesWithScores[j]), std::get<0>(validMoves[j]));
            EXPECT_EQ(std::get<1>(validMovesWithScores[j]), std::get<1>(validMoves[j]));
        }
    }
}

// Test specific move scenarios
TEST_F(BoardTest, SpecificMoveScenarioTest) {
    // Scenario 1: Multiple merges in a row
    std::vector<std::vector<int>> tiles = {
        {2, 2, 2, 2},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    uint64_t state = createBoardState(tiles);
    
    // Simulate LEFT move
    auto moves = Board::simulateMovesWithScores(state);
    uint64_t leftState = std::get<0>(moves[0]);
    int leftScore = std::get<1>(moves[0]);
    
    // Verify that the board has changed after the move
    EXPECT_NE(leftState, state);
    
    // Verify that the score is positive
    EXPECT_GT(leftScore, 0);
    
    // Scenario 2: Merges with different values
    tiles = {
        {2, 2, 4, 4},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    state = createBoardState(tiles);
    
    // Simulate LEFT move
    moves = Board::simulateMovesWithScores(state);
    leftState = std::get<0>(moves[0]);
    leftScore = std::get<1>(moves[0]);
    
    // Verify that the board has changed after the move
    EXPECT_NE(leftState, state);
    
    // Verify that the score is positive
    EXPECT_GT(leftScore, 0);
}

/**
 * @brief Tests the Board::transpose function
 * 
 * Verifies the transpose operation correctly rearranges the board
 * by swapping rows and columns.
 */
TEST_F(BoardTest, TransposeOperation) {
    // Print values in hex for easier debugging
    auto printHex = [](uint64_t val) -> std::string {
        std::stringstream ss;
        ss << "0x" << std::hex << std::uppercase << val;
        return ss.str();
    };

    // Test case 1: Empty board
    uint64_t emptyBoard = 0x0ULL;
    EXPECT_EQ(Board::transpose(emptyBoard), emptyBoard)
        << "Empty board: expected " << printHex(emptyBoard)
        << ", got " << printHex(Board::transpose(emptyBoard));
    
    // Test case 2: Single tile at position (0,0) with value 2 (internal value 1)
    uint64_t singleTile = 0x1ULL;
    EXPECT_EQ(Board::transpose(singleTile), singleTile)
        << "Single tile: expected " << printHex(singleTile)
        << ", got " << printHex(Board::transpose(singleTile));
    
    // Test case 3: Single tile at position (0,1) should move to (1,0)
    // (0,1) = 4 bits offset = 0x10, (1,0) = 16 bits offset = 0x10000
    uint64_t tile01 = 0x10ULL;
    uint64_t tile10 = 0x10000ULL;
    EXPECT_EQ(Board::transpose(tile01), tile10)
        << "Tile at (0,1): expected " << printHex(tile10)
        << ", got " << printHex(Board::transpose(tile01));
    EXPECT_EQ(Board::transpose(tile10), tile01)
        << "Tile at (1,0): expected " << printHex(tile01)
        << ", got " << printHex(Board::transpose(tile10));
    
    // Test case 4: Diagonal pattern - should remain unchanged
    // Set tiles at (0,0), (1,1), (2,2), (3,3) to values 1,2,3,4
    uint64_t diagonal = 0x4000030000200001ULL;
    EXPECT_EQ(Board::transpose(diagonal), diagonal)
        << "Diagonal: expected " << printHex(diagonal)
        << ", got " << printHex(Board::transpose(diagonal));
    
    // Test case 5: Row pattern - should become column pattern
    // Row 0: [1,2,3,4] -> Column 0: [1,2,3,4] but in column orientation
    uint64_t firstRow = 0x4321ULL;
    uint64_t firstCol = 0x4000300020001ULL;  // Corrected value
    EXPECT_EQ(Board::transpose(firstRow), firstCol)
        << "First row: expected " << printHex(firstCol)
        << ", got " << printHex(Board::transpose(firstRow));
    
    // Test case 6: Complex pattern
    // 2 4 0 0     2 8 0 0
    // 8 0 0 0  -> 4 0 0 0
    // 0 0 0 0     0 0 0 0
    // 0 0 0 0     0 0 0 0
    uint64_t pattern1 = 0x30021ULL;  // Corrected value
    uint64_t pattern2 = 0x20031ULL;  // Corrected value
    EXPECT_EQ(Board::transpose(pattern1), pattern2)
        << "Complex pattern: expected " << printHex(pattern2)
        << ", got " << printHex(Board::transpose(pattern1));
    
    // Test case 7: Double transposition should return original
    uint64_t randomState = 0x0123456789ABCDEFULL;
    EXPECT_EQ(Board::transpose(Board::transpose(randomState)), randomState)
        << "Double transpose: expected " << printHex(randomState)
        << ", got " << printHex(Board::transpose(Board::transpose(randomState)));
    
    // Test case 8: Full board with varied values
    uint64_t fullBoard = 0xFEDCBA9876543210ULL;
    uint64_t transposedFullBoard = Board::transpose(fullBoard);
    EXPECT_EQ(Board::transpose(transposedFullBoard), fullBoard)
        << "Full board double transpose: expected " << printHex(fullBoard)
        << ", got " << printHex(Board::transpose(transposedFullBoard));
    
    // Visual debugging
    std::cout << "\nDiagonal board:" << std::endl;
    printBoardState(diagonal);
    std::cout << "After transpose:" << std::endl;
    printBoardState(Board::transpose(diagonal));
    
    std::cout << "\nRow pattern:" << std::endl;
    printBoardState(firstRow);
    std::cout << "After transpose (should be column):" << std::endl;
    printBoardState(Board::transpose(firstRow));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 