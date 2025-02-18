
// board.cpp
#include "board.hpp"
#include <algorithm>

bool Board::lookupInitialized = false;
uint16_t Board::leftMoves[1 << 16];
uint16_t Board::rightMoves[1 << 16];

uint16_t Board::moveLeft(uint16_t row) {
    int values[4] = {
        (row >> 12) & 0xF,
        (row >> 8) & 0xF,
        (row >> 4) & 0xF,
        row & 0xF
    };

    // Remove zeros and merge
    std::vector<int> nonZero;
    for (int val : values) {
        if (val != 0) nonZero.push_back(val);
    }

    std::vector<int> result(4, 0);
    int j = 0;
    for (size_t i = 0; i < nonZero.size(); ++i) {
        if (i + 1 < nonZero.size() && nonZero[i] == nonZero[i + 1]) {
            if (nonZero[i] == 15) {
                result[j++] = nonZero[i];
                result[j++] = nonZero[i];
                ++i;
            } else {
                result[j++] = nonZero[i] + 1;
                ++i;
            }
        } else {
            result[j++] = nonZero[i];
        }
    }

    return (result[0] << 12) | (result[1] << 8) | (result[2] << 4) | result[3];
}

uint16_t Board::moveRight(uint16_t row) {
    uint16_t reversed = ((row & 0xF) << 12) | ((row & 0xF0) << 4) |
                       ((row & 0xF00) >> 4) | ((row & 0xF000) >> 12);
    uint16_t moved = moveLeft(reversed);
    return ((moved & 0xF) << 12) | ((moved & 0xF0) << 4) |
           ((moved & 0xF00) >> 4) | ((moved & 0xF000) >> 12);
}

void Board::initLookupTables() {
    for (int i = 0; i < (1 << 16); ++i) {
        leftMoves[i] = moveLeft(i);
        rightMoves[i] = moveRight(i);
    }
}

std::vector<std::tuple<int, int>> Board::getEmptyTiles(uint64_t state) {
    std::vector<std::tuple<int, int>> empty;
    for (int i = 0; i < 16; ++i) {
        if (((state >> (i * 4)) & 0xF) == 0) {
            empty.emplace_back(i / 4, i % 4);
        }
    }
    return empty;
}

uint64_t Board::setTile(uint64_t state, int row, int col, int value) {
    int pos = (row * 4 + col) * 4;
    return state | (static_cast<uint64_t>(value) << pos);
}

std::vector<uint64_t> Board::simulateMoves(uint64_t state) {
    std::vector<uint64_t> results(4, 0);

    // Horizontal moves
    for (int row = 0; row < 4; ++row) {
        uint16_t rowVal = (state >> (16 * row)) & 0xFFFF;
        results[0] |= static_cast<uint64_t>(leftMoves[rowVal]) << (16 * row);
        results[1] |= static_cast<uint64_t>(rightMoves[rowVal]) << (16 * row);
    }

    // Vertical moves
    uint16_t cols[4] = {0};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            int shift = (3 - row) * 4;
            cols[col] |= ((state >> ((row * 16) + (12 - col * 4))) & 0xF) << shift;
        }
    }

    for (int col = 0; col < 4; ++col) {
        uint16_t upMove = rightMoves[cols[col]];
        uint16_t downMove = leftMoves[cols[col]];

        for (int row = 0; row < 4; ++row) {
            results[2] |= static_cast<uint64_t>((upMove >> (12 - row * 4)) & 0xF) << ((row * 16) + (12 - col * 4));
            results[3] |= static_cast<uint64_t>((downMove >> (12 - row * 4)) & 0xF) << ((row * 16) + (12 - col * 4));
        }
    }

    return results;
}

std::vector<std::tuple<Action, uint64_t>> Board::getValidMoveActions(uint64_t state) {
    std::vector<std::tuple<Action, uint64_t>> valid;
    auto moves = simulateMoves(state);

    for (int i = 0; i < 4; ++i) {
        if (moves[i] != state) {
            valid.emplace_back(static_cast<Action>(i), moves[i]);
        }
    }

    return valid;
}