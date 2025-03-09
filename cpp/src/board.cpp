// board.cpp
#include "board.hpp"
#include <algorithm>
#include <array>
#include <iostream>

bool Board::lookupInitialized = false;
std::array<uint16_t, 1 << 16> Board::leftMoves;
std::array<uint16_t, 1 << 16> Board::rightMoves;
std::array<int, 1 << 16> Board::leftScores;
std::array<int, 1 << 16> Board::rightScores;

std::string actionToString(Action action) {
    switch (action) {
        case Action::LEFT:
            return "LEFT";
        case Action::RIGHT:
            return "RIGHT";
        case Action::UP:
            return "UP";
        case Action::DOWN:
            return "DOWN";
        default:
            return "INVALID";
    }
}

uint16_t Board::moveLeft(uint16_t row, int& score) {
    int values[4] = {
        (row >> 12) & 0xF,
        (row >> 8) & 0xF,
        (row >> 4) & 0xF,
        row & 0xF
    };

    // Reset score
    score = 0;

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
                // When merging, add the score: 2^(value+1)
                int mergedValue = nonZero[i] + 1;
                result[j++] = mergedValue;
                score += valueToTile(mergedValue);
                ++i;
            }
        } else {
            result[j++] = nonZero[i];
        }
    }

    return static_cast<uint16_t>((result[0] << 12) | (result[1] << 8) | (result[2] << 4) | result[3]);
}

uint16_t Board::moveRight(uint16_t row, int& score) {
    uint16_t reversed = ((row & 0xF) << 12) | ((row & 0xF0) << 4) |
                       ((row & 0xF00) >> 4) | ((row & 0xF000) >> 12);
    uint16_t moved = moveLeft(reversed, score);
    return ((moved & 0xF) << 12) | ((moved & 0xF0) << 4) |
           ((moved & 0xF00) >> 4) | ((moved & 0xF000) >> 12);
}

int Board::countEmptyTiles(uint16_t row) {
    int count = 0;
    for (int i = 0; i < 4; ++i) {
        if (((row >> (i * 4)) & 0xF) == 0) {
            count++;
        }
    }
    return count;
}

void Board::initLookupTables() {
    for (int i = 0; i < (1 << 16); ++i) {
        int leftScore = 0;
        int rightScore = 0;
        leftMoves[i] = static_cast<uint16_t>(moveLeft(static_cast<uint16_t>(i), leftScore));
        rightMoves[i] = static_cast<uint16_t>(moveRight(static_cast<uint16_t>(i), rightScore));
        leftScores[i] = leftScore;
        rightScores[i] = rightScore;
    }
}

std::vector<std::tuple<int, int>> Board::getEmptyTiles(uint64_t state) {
    std::vector<std::tuple<int, int>> empty;
    empty.reserve(16);  // Pre-allocate maximum possible size

    // Use 16-bit word processing
    for (int row = 0; row < 4; ++row) {
        uint16_t rowVal = (state >> (row * 16)) & 0xFFFF;

        for (int col = 0; col < 4; ++col) {
            if (((rowVal >> (col * 4)) & 0xF) == 0) {
                empty.emplace_back(row, col);
            }
        }
    }
    return empty;
}

uint64_t Board::setTile(uint64_t state, int row, int col, int value) {
    int pos = (row * 4 + col) * 4;
    return state | (static_cast<uint64_t>(value) << pos);
}

std::vector<std::tuple<uint64_t, int>> Board::simulateMovesWithScores(uint64_t state) {
    std::vector<std::tuple<uint64_t, int>> results(4);

    // Initialize all states to the original state and scores to 0
    for (int i = 0; i < 4; ++i) {
        std::get<0>(results[i]) = 0;
        std::get<1>(results[i]) = 0;
    }

    // Vertical moves
    uint64_t transposedState = transpose(state);
    // Horizontal moves
    for (int idx = 0; idx < 4; ++idx) {
        uint64_t shift = 16 * idx;
        uint16_t rowVal = (state >> shift) & 0xFFFF;
        std::get<0>(results[0]) |= static_cast<uint64_t>(leftMoves[rowVal]) << shift;
        std::get<1>(results[0]) += leftScores[rowVal];

        std::get<0>(results[1]) |= static_cast<uint64_t>(rightMoves[rowVal]) << shift;
        std::get<1>(results[1]) += rightScores[rowVal];

        uint16_t colVal = (transposedState >> shift) & 0xFFFF;
        std::get<0>(results[2]) |= static_cast<uint64_t>(leftMoves[colVal]) << shift;
        std::get<1>(results[2]) += leftScores[colVal];

        std::get<0>(results[3]) |= static_cast<uint64_t>(rightMoves[colVal]) << shift;
        std::get<1>(results[3]) += rightScores[colVal];
    }

    std::get<0>(results[2]) = transpose(std::get<0>(results[2]));
    std::get<0>(results[3]) = transpose(std::get<0>(results[3]));

    return results;
}

std::vector<ChosenActionResult> Board::getValidMoveActionsWithScores(uint64_t state) {
    std::vector<ChosenActionResult> valid;
    auto moves = simulateMovesWithScores(state);

    for (int i = 0; i < 4; ++i) {
        if (std::get<0>(moves[i]) != state) {
            valid.emplace_back(static_cast<Action>(i), std::get<0>(moves[i]), std::get<1>(moves[i]));
        }
    }

    return valid;
}

std::vector<std::tuple<Action, uint64_t>> Board::getValidMoveActions(uint64_t state) {
    auto movesWithScores = getValidMoveActionsWithScores(state);
    std::vector<std::tuple<Action, uint64_t>> valid;

    for (const auto& [action, nextState, score] : movesWithScores) {
        valid.emplace_back(action, nextState);
    }

    return valid;
}

/**
 * @brief Transposes the board (converts rows to columns and vice versa)
 *
 * Uses efficient bit manipulation to transpose the 4x4 board by rearranging
 * the 4-bit tiles within the 64-bit state.
 *
 * @param state 64-bit board state
 * @return uint64_t Transposed board state
 */
uint64_t Board::transpose(uint64_t state) {
    uint64_t a1 = state & 0xF0F00F0FF0F00F0FULL;
    uint64_t a2 = state & 0x0000F0F00000F0F0ULL;
    uint64_t a3 = state & 0x0F0F00000F0F0000ULL;
    uint64_t a = a1 | (a2 << 12) | (a3 >> 12);
    uint64_t b1 = a & 0xFF00FF0000FF00FFULL;
    uint64_t b2 = a & 0x00FF00FF00000000ULL;
    uint64_t b3 = a & 0x00000000FF00FF00ULL;
    return b1 | (b2 >> 24) | (b3 << 24);
}

void Board::printBoard(uint8_t board[4][4]) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << (int)board[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}
