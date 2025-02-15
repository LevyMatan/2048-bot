// game.cpp
#include "game.hpp"
#include <iostream>
#include <iomanip>

void Game2048::addRandomTile() {
    auto emptyTiles = Board::getEmptyTiles(board.getState());
    if (emptyTiles.empty()) return;

    int idx = rng() % emptyTiles.size();
    auto [row, col] = emptyTiles[idx];
    int value = (dist(rng) < 0.9) ? 1 : 2;

    board.setState(Board::setTile(board.getState(), row, col, value));
}

bool Game2048::playMove() {
    auto validActions = Board::getValidMoveActions(board.getState());
    if (validActions.empty()) return false;

    // Find move with best heuristic score
    double bestScore = -1;
    uint64_t bestNextState = std::get<1>(validActions[0]);

    for (const auto& action : validActions) {
        uint64_t nextState = std::get<1>(action);
        double score = evaluatePosition(nextState);
        if (score > bestScore) {
            bestScore = score;
            bestNextState = nextState;
        }
    }

    board.setState(bestNextState);
    addRandomTile();
    return true;
}

double Game2048::evaluatePosition(uint64_t state) const {
    double score = 0.0;

    // Weight for empty tiles (0.2)
    score += 0.2 * Board::getEmptyTiles(state).size();

    // Weight for monotonicity (0.4)
    score += 0.4 * evaluateMonotonicity(state);

    // Weight for smoothness (0.2)
    score += 0.2 * evaluateSmoothness(state);

    // Weight for corner placement (0.2)
    score += 0.2 * evaluateCornerPlacement(state);

    return score;
}

double Game2048::evaluateMonotonicity(uint64_t state) const {
    double score = 0.0;
    // Check rows and columns for monotonicity
    for (int i = 0; i < 4; ++i) {
        bool increasing = true;
        bool decreasing = true;

        // Check horizontal monotonicity
        for (int j = 0; j < 3; ++j) {
            int current = (state >> ((15-i*4-j) * 4)) & 0xF;
            int next = (state >> ((15-i*4-j-1) * 4)) & 0xF;
            if (current > next) increasing = false;
            if (current < next) decreasing = false;
        }
        score += increasing || decreasing ? 1.0 : 0.0;

        // Reset flags for vertical check
        increasing = true;
        decreasing = true;

        // Check vertical monotonicity
        for (int j = 0; j < 3; ++j) {
            int current = (state >> ((15-i-j*4) * 4)) & 0xF;
            int next = (state >> ((15-i-(j+1)*4) * 4)) & 0xF;
            if (current > next) increasing = false;
            if (current < next) decreasing = false;
        }
        score += increasing || decreasing ? 1.0 : 0.0;
    }
    return score / 8.0; // Normalize score to [0,1] range
}

double Game2048::evaluateSmoothness(uint64_t state) const {
    double score = 0.0;
    // Check adjacent tiles
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int current = (state >> ((15-i*4-j) * 4)) & 0xF;
            if (current == 0) continue;

            // Check right neighbor
            if (j < 3) {
                int right = (state >> ((15-i*4-j-1) * 4)) & 0xF;
                if (right != 0) {
                    score += 1.0 / (1.0 + std::abs(current - right));
                }
            }
            // Check bottom neighbor
            if (i < 3) {
                int bottom = (state >> ((15-(i+1)*4-j) * 4)) & 0xF;
                if (bottom != 0) {
                    score += 1.0 / (1.0 + std::abs(current - bottom));
                }
            }
        }
    }
    return score;
}

double Game2048::evaluateCornerPlacement(uint64_t state) const {
    double score = 0.0;
    int maxValue = 0;
    int maxPos = -1;

    // Find max value and its position
    for (int i = 0; i < 16; ++i) {
        int value = (state >> (i * 4)) & 0xF;
        if (value > maxValue) {
            maxValue = value;
            maxPos = i;
        }
    }

    // Bonus for max value in corners
    if (maxPos == 0 || maxPos == 3 || maxPos == 12 || maxPos == 15) {
        score += 2.0;
    }

    return score;
}

int Game2048::getScore() const {
    int score = 0;
    uint64_t state = board.getState();
    for (int i = 0; i < 16; ++i) {
        int value = (state >> (i * 4)) & 0xF;
        if (value > 0) {
            score += 1 << value;
        }
    }
    return score;
}

void Game2048::reset() {
    board.setState(0);
    moveCount = 0;
    addRandomTile();
    addRandomTile();
}

std::tuple<int, uint64_t, int> Game2048::playGame() {
    reset();
    while (playMove()) {
        moveCount++;
    }
    return {getScore(), board.getState(), moveCount};
}

void Game2048::prettyPrint() const {
    std::cout << std::string(11, '-') << '\n';
    uint64_t state = board.getState();
    for (int i = 0; i < 4; ++i) {
        uint16_t row = (state >> (48 - i * 16)) & 0xFFFF;
        std::cout << std::hex << std::uppercase
                  << ((row >> 12) & 0xF) << ' '
                  << ((row >> 8) & 0xF) << ' '
                  << ((row >> 4) & 0xF) << ' '
                  << (row & 0xF) << '\n';
    }
    std::cout << std::dec;
}