// board.cpp
#include "board.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <iomanip>
#include <random>

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

uint16_t Board::moveRight(uint16_t row, int& score) {
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

uint16_t Board::moveLeft(uint16_t row, int& score) {
    uint16_t reversed = ((row & 0xF) << 12) | ((row & 0xF0) << 4) |
                       ((row & 0xF00) >> 4) | ((row & 0xF000) >> 12);
    uint16_t moved = moveRight(reversed, score);
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

std::vector<std::tuple<int, int>> Board::getEmptyTiles(BoardState state) {
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

BoardState Board::setTile(BoardState state, int row, int col, int value) {
    int pos = (row * 4 + col) * 4;
    return state | (static_cast<BoardState>(value) << pos);
}

std::vector<std::tuple<BoardState, int>> Board::simulateMovesWithScores(BoardState state) {
    std::vector<std::tuple<BoardState, int>> results(4);

    // Initialize all states to the original state and scores to 0
    for (int i = 0; i < 4; ++i) {
        std::get<0>(results[i]) = 0;
        std::get<1>(results[i]) = 0;
    }

    // Vertical moves
    BoardState transposedState = transpose(state);
    // Horizontal moves
    for (int idx = 0; idx < 4; ++idx) {
        BoardState shift = 16 * idx;
        uint16_t rowVal = (state >> shift) & 0xFFFF;
        std::get<0>(results[0]) |= static_cast<BoardState>(leftMoves[rowVal]) << shift;
        std::get<1>(results[0]) += leftScores[rowVal];

        std::get<0>(results[1]) |= static_cast<BoardState>(rightMoves[rowVal]) << shift;
        std::get<1>(results[1]) += rightScores[rowVal];

        uint16_t colVal = (transposedState >> shift) & 0xFFFF;
        std::get<0>(results[2]) |= static_cast<BoardState>(leftMoves[colVal]) << shift;
        std::get<1>(results[2]) += leftScores[colVal];

        std::get<0>(results[3]) |= static_cast<BoardState>(rightMoves[colVal]) << shift;
        std::get<1>(results[3]) += rightScores[colVal];
    }

    std::get<0>(results[2]) = transpose(std::get<0>(results[2]));
    std::get<0>(results[3]) = transpose(std::get<0>(results[3]));

    return results;
}

std::vector<ChosenActionResult> Board::getValidMoveActionsWithScores(BoardState state) {
    std::vector<ChosenActionResult> valid;
    auto moves = simulateMovesWithScores(state);

    for (int i = 0; i < 4; ++i) {
        if (std::get<0>(moves[i]) != state) {
            valid.emplace_back(static_cast<Action>(i), std::get<0>(moves[i]), std::get<1>(moves[i]));
        }
    }

    return valid;
}

std::vector<std::tuple<Action, BoardState>> Board::getValidMoveActions(BoardState state) {
    auto movesWithScores = getValidMoveActionsWithScores(state);
    std::vector<std::tuple<Action, BoardState>> valid;

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
 * @return BoardState Transposed board state
 */
BoardState Board::transpose(BoardState state) {
    BoardState a1 = state & 0xF0F00F0FF0F00F0FULL;
    BoardState a2 = state & 0x0000F0F00000F0F0ULL;
    BoardState a3 = state & 0x0F0F00000F0F0000ULL;
    BoardState a = a1 | (a2 << 12) | (a3 >> 12);
    BoardState b1 = a & 0xFF00FF0000FF00FFULL;
    BoardState b2 = a & 0x00FF00FF00000000ULL;
    BoardState b3 = a & 0x00000000FF00FF00ULL;
    return b1 | (b2 >> 24) | (b3 << 24);
}

// Helper function to unpack state into a 2D board
void Board::unpackState(BoardState state, uint8_t board[4][4]) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            board[row][col] = static_cast<uint8_t>(
                (state >> ((row * 4 + col) * 4)) & 0xF
            );
        }
    }
}

void Board::printBoard(uint8_t board[4][4]) {
    std::cout << "+------+------+------+------+" << std::endl;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << "| " << std::setw(5) << (board[i][j] ? std::to_string(board[i][j]) : " ");
        }
        std::cout << "|" << std::endl;
        std::cout << "+------+------+------+------+" << std::endl;
    }
}

BoardState Board::randomizeState() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> probDist(0.0, 1.0);

    BoardState state = 0;
    // There are 16 tiles on the board
    for (int i = 0; i < 16; ++i) {
        double r = probDist(gen);
        int tile;
        if (r < 0.5) {
            tile = 0;
        } else if (r < 0.8) { // 0.3 probability
            std::uniform_int_distribution<int> dist(1, 10);
            tile = dist(gen);
        } else if (r < 0.95) { // 0.15 probability
            std::uniform_int_distribution<int> dist(11, 12);
            tile = dist(gen);
        } else { // remaining 0.05 probability
            std::uniform_int_distribution<int> dist(13, 15);
            tile = dist(gen);
        }
        // Each tile is stored in 4 bits in the 64-bit state.
        state |= (static_cast<BoardState>(tile) << (i * 4));
    }

    return state;
}
