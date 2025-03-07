// board.hpp
#pragma once
#include <stdint.h>
#include <vector>
#include <tuple>
#include <array>

/**
 * @brief Represents the four possible move directions in the 2048 game
 */
enum class Action {
    LEFT = 0,
    RIGHT = 1,
    UP = 2,
    DOWN = 3,
    INVALID = 4
};

std::string actionToString(Action action);

using BoardState = uint64_t;
class Board {
private:
    BoardState state;
    static bool lookupInitialized;
    static std::array<uint16_t, 1 << 16> leftMoves;
    static std::array<uint16_t, 1 << 16> rightMoves;
    static std::array<int, 1 << 16> leftScores;
    static std::array<int, 1 << 16> rightScores;

    /**
     * @brief Initializes lookup tables for efficient move calculations
     *
     * Creates precomputed tables for all possible 16-bit row configurations to
     * optimize game calculations. This is called once during the first Board instantiation.
     */
    static void initLookupTables();

    /**
     * @brief Calculates the result of moving a single row left
     *
     * @param row 16-bit row representation
     * @param score Reference to score value that will be updated with points earned
     * @return uint16_t New row state after left move
     */
    static uint16_t moveLeft(uint16_t row, int& score);

    /**
     * @brief Calculates the result of moving a single row right
     *
     * @param row 16-bit row representation
     * @param score Reference to score value that will be updated with points earned
     * @return uint16_t New row state after right move
     */
    static uint16_t moveRight(uint16_t row, int& score);

    static int countEmptyTiles(uint16_t row);

public:
    /**
     * @brief Transposes the board (converts rows to columns and vice versa)
     *
     * Uses efficient bit manipulation to transpose the 4x4 board by rearranging
     * the 4-bit tiles within the 64-bit state.
     *
     * @param state 64-bit board state
     * @return uint64_t Transposed board state
     */
    static uint64_t transpose(uint64_t state);
    /**
     * @brief Constructs a new empty Board
     *
     * Initializes the board with all tiles set to 0 and ensures lookup tables
     * are initialized on first instantiation.
     */
    Board() : state(0) {
        if (!lookupInitialized) {
            initLookupTables();
            lookupInitialized = true;
        }
    }

    /**
     * @brief Sets the board state to a new value
     *
     * @param newState 64-bit integer representing the new board state
     */
    void setState(uint64_t newState) { state = newState; }

    /**
     * @brief Gets the current board state
     *
     * @return uint64_t 64-bit integer representing the current board state
     */
    uint64_t getState() const { return state; }

    /**
     * @brief Finds all empty tiles on the board
     *
     * @param state 64-bit board state
     * @return std::vector<std::tuple<int, int>> Vector of (row, column) coordinates of empty tiles
     */
    static std::vector<std::tuple<int, int>> getEmptyTiles(uint64_t state);

    /**
     * @brief Gets all valid moves with their resulting states and scores
     *
     * @param state 64-bit board state
     * @return std::vector<std::tuple<Action, uint64_t, int>> Vector of (action, new state, score) tuples
     */
    static std::vector<std::tuple<Action, uint64_t, int>> getValidMoveActionsWithScores(uint64_t state);

    /**
     * @brief Sets a specific tile to a value
     *
     * @param state 64-bit board state
     * @param row Row coordinate (0-3)
     * @param col Column coordinate (0-3)
     * @param value Value to set (in internal representation)
     * @return uint64_t New board state after setting the tile
     */
    static uint64_t setTile(uint64_t state, int row, int col, int value);

    /**
     * @brief Simulates all possible moves and returns resulting states with scores
     *
     * @param state 64-bit board state
     * @return std::vector<std::tuple<uint64_t, int>> Vector of (new state, score) tuples
     */
    static std::vector<std::tuple<uint64_t, int>> simulateMovesWithScores(uint64_t state);

    /**
     * @brief Gets all valid moves with their resulting states
     *
     * @param state 64-bit board state
     * @return std::vector<std::tuple<Action, uint64_t>> Vector of (action, new state) tuples
     * @note Maintained for backward compatibility
     */
    static std::vector<std::tuple<Action, uint64_t>> getValidMoveActions(uint64_t state);

    /**
     * @brief Converts a tile value to its internal representation
     *
     * Converts the actual tile value (2, 4, 8, etc.) to its internal
     * representation as a power of 2 (1, 2, 3, etc.)
     *
     * @param tile Actual tile value (2^n)
     * @return int Internal representation (n)
     */
    static int tileToValue(int tile) {
        int value = 0;
        while (tile > 1) {
            tile >>= 1;
            value++;
        }
        return value;
    }

    /**
     * @brief Converts internal representation to actual tile value
     *
     * Converts the internal representation (1, 2, 3, etc.) to the
     * actual tile value (2, 4, 8, etc.)
     *
     * @param value Internal representation (n)
     * @return int Actual tile value (2^n)
     */
    static int valueToTile(int value) {
        return 1 << value;
    }

    /**
     * @brief Get the value of a tile at the specified position
     * @param state The 64-bit board state
     * @param row Row coordinate (0-3)
     * @param col Column coordinate (0-3)
     * @return The value at the specified position (0 if empty)
     */
    static int getTileAt(uint64_t state, int row, int col) {
        int pos = (row * 4 + col) * 4;
        return (state >> pos) & 0xF;
    }

    /**
     * @brief Calculates the score for a given board state
     *
     * @param state 64-bit board state
     * @return uint64_t Score calculated from the board state
     */
    static uint64_t getScore(uint64_t state) {
        uint64_t score = 0;

        // Sum the values of all tiles
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                int tileValue = getTileAt(state, row, col);
                if (tileValue > 1) {
                    // Score is 2^tile, but only for tiles > 2 (value > 1)
                    score += (1ULL << tileValue);
                }
            }
        }

        return score;
    }

    /**
     * @brief Prints the board to the console
     *
     * @param board 4x4 array representing the board
     */
    static void printBoard(uint8_t board[4][4]);
};

/**
 * @brief Board class for the 2048 game
 *
 * This class represents a 2048 game board and provides functionality for game mechanics.
 *
 * ## State Representation
 * The game state is stored as a 64-bit integer (uint64_t) where:
 * - Each tile uses 4 bits to store its value
 * - Values are stored as powers of 2 (e.g., 2^1=2, 2^2=4, 2^3=8, etc.)
 * - A 4x4 board requires 16 tiles * 4 bits = 64 bits total
 *
 * ## Board Layout
 * The 4x4 board is mapped to the 64-bit integer as follows:
 * [0,0][0,1][0,2][0,3]
 * [1,0][1,1][1,2][1,3]
 * [2,0][2,1][2,2][2,3]
 * [3,0][3,1][3,2][3,3]
 *
 * Each [row,col] position represents a 4-bit value in the state integer.
 *
 * ## Actions
 * The game supports four basic movements: LEFT, RIGHT, UP, and DOWN as defined
 * in the Action enum.
 *
 * ## Key Methods
 * - getValidMoveActions: Returns all valid moves from current state
 * - getValidMoveActionsWithScores: Returns valid moves with their resulting scores
 * - simulateMovesWithScores: Simulates moves and returns resulting scores
 * - getEmptyTiles: Returns coordinates of all empty tiles
 * - setTile: Sets a specific tile value at given coordinates
 *
 * The internal representation uses log base 2 of the actual tile values to save space.
 * Helper methods tileToValue and valueToTile handle the conversion between these representations.
 */
