// board.hpp
#pragma once
#include <stdint.h>
#include <vector>
#include <tuple>

enum class Action {
    LEFT = 0,
    RIGHT = 1,
    UP = 2,
    DOWN = 3
};

class Board {
private:
    uint64_t state;
    static bool lookupInitialized;
    static uint16_t leftMoves[1 << 16];
    static uint16_t rightMoves[1 << 16];
    static int leftScores[1 << 16];
    static int rightScores[1 << 16];

    static void initLookupTables();
    static uint16_t moveLeft(uint16_t row, int& score);
    static uint16_t moveRight(uint16_t row, int& score);

public:
    Board() : state(0) {
        if (!lookupInitialized) {
            initLookupTables();
            lookupInitialized = true;
        }
    }

    void setState(uint64_t newState) { state = newState; }
    uint64_t getState() const { return state; }

    static std::vector<std::tuple<int, int>> getEmptyTiles(uint64_t state);
    static std::vector<std::tuple<Action, uint64_t, int>> getValidMoveActionsWithScores(uint64_t state);
    static uint64_t setTile(uint64_t state, int row, int col, int value);
    static std::vector<std::tuple<uint64_t, int>> simulateMovesWithScores(uint64_t state);
    
    // Keep the original methods for backward compatibility
    static std::vector<uint64_t> simulateMoves(uint64_t state);
    static std::vector<std::tuple<Action, uint64_t>> getValidMoveActions(uint64_t state);
    
    // Helper to convert tile value (2^n) to internal representation (n)
    static int tileToValue(int tile) {
        int value = 0;
        while (tile > 1) {
            tile >>= 1;
            value++;
        }
        return value;
    }
    
    // Helper to convert internal representation (n) to tile value (2^n)
    static int valueToTile(int value) {
        return 1 << value;
    }
};
