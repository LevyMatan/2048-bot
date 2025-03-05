#include "expectimax_player.hpp"
#include <chrono>
#include <algorithm>
#include <limits>

ExpectimaxPlayer::ExpectimaxPlayer(const Config& cfg, EvaluationFunction fn)
    : config(cfg),
      evalFn(fn ? fn : defaultEvaluation),
      startTime(std::chrono::steady_clock::now()) {}

std::tuple<Action, uint64_t, int> ExpectimaxPlayer::chooseAction(uint64_t state) {
    startTime = std::chrono::steady_clock::now();

    int searchDepth = config.adaptiveDepth ?
                     getAdaptiveDepth(state) : config.depth;

    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return {Action::INVALID, state, 0};
    }

    Action bestAction = Action::INVALID;
    uint64_t bestState = state;
    double bestScore = -std::numeric_limits<double>::infinity();
    int bestMoveScore = 0;

    for (const auto& [action, nextState, moveScore] : validMoves) {
        double score = chanceNode(nextState, searchDepth - 1);
        if (score > bestScore) {
            bestScore = score;
            bestAction = action;
            bestState = nextState;
            bestMoveScore = moveScore;
        }

        if (shouldTimeOut()) {
            break;
        }
    }

    return {bestAction, bestState, bestMoveScore};
}

double ExpectimaxPlayer::expectimax(uint64_t state, int depth, bool isMax) {
    if (depth == 0 || shouldTimeOut()) {
        return evalFn(state);
    }
    return isMax ? maxNode(state, depth) : chanceNode(state, depth);
}

double ExpectimaxPlayer::chanceNode(uint64_t state, int depth) {
    if (depth <= 0 || shouldTimeOut()) {
        return static_cast<double>(evalFn(state));
    }

    auto emptyTiles = Board::getEmptyTiles(state);
    if (emptyTiles.empty()) {
        return static_cast<double>(evalFn(state));
    }

    double totalScore = 0.0;
    int numSamples = std::min(config.chanceCovering,
                             static_cast<int>(emptyTiles.size()));

    for (int i = 0; i < numSamples && !shouldTimeOut(); ++i) {
        auto [row, col] = emptyTiles[i % emptyTiles.size()];

        // 90% chance of 2 (value 1), 10% chance of 4 (value 2)
        uint64_t newState2 = Board::setTile(state, row, col, 1);
        uint64_t newState4 = Board::setTile(state, row, col, 2);

        totalScore += 0.9 * maxNode(newState2, depth - 1);
        totalScore += 0.1 * maxNode(newState4, depth - 1);
    }

    return totalScore / numSamples;
}

double ExpectimaxPlayer::maxNode(uint64_t state, int depth) {
    if (depth <= 0 || shouldTimeOut()) {
        return static_cast<double>(evalFn(state));
    }

    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return static_cast<double>(evalFn(state));
    }

    double bestScore = -std::numeric_limits<double>::infinity();

    for (const auto& [action, nextState, moveScore] : validMoves) {
        bestScore = std::max(bestScore,
                           chanceNode(nextState, depth - 1) + moveScore);

        if (shouldTimeOut()) {
            break;
        }
    }

    return bestScore;
}

bool ExpectimaxPlayer::shouldTimeOut() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                   (now - startTime).count() / 1000.0;
    return duration > config.timeLimit;
}

int ExpectimaxPlayer::getAdaptiveDepth(uint64_t state) const {
    int emptyCount = Board::getEmptyTileCount(state);
    // Adjust depth based on number of empty tiles
    if (emptyCount <= 4) return config.depth + 2;
    if (emptyCount <= 8) return config.depth + 1;
    return config.depth;
}

// Evaluation functions
uint64_t ExpectimaxPlayer::defaultEvaluation(uint64_t state) {
    uint64_t score = 0;
    score += Board::getEmptyTileCount(state) * 100;  // Empty tiles weight

    // Add surface smoothness
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            int curr = Board::getTileAt(state, row, col);
            int next = Board::getTileAt(state, row, col + 1);
            if (curr > 0 && next > 0) {
                score += (curr == next) ? 50 : 0;
            }
        }
    }

    return score;
}

uint64_t ExpectimaxPlayer::monotonicEvaluation(uint64_t state) {
    uint64_t score = 0;

    // Check horizontal monotonicity
    for (int row = 0; row < 4; ++row) {
        bool increasing = true;
        bool decreasing = true;
        for (int col = 1; col < 4; ++col) {
            int prev = Board::getTileAt(state, row, col-1);
            int curr = Board::getTileAt(state, row, col);
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
        }
        if (increasing || decreasing) score += 200;
    }

    // Check vertical monotonicity
    for (int col = 0; col < 4; ++col) {
        bool increasing = true;
        bool decreasing = true;
        for (int row = 1; row < 4; ++row) {
            int prev = Board::getTileAt(state, row-1, col);
            int curr = Board::getTileAt(state, row, col);
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
        }
        if (increasing || decreasing) score += 200;
    }

    return score;
}

uint64_t ExpectimaxPlayer::cornerEvaluation(uint64_t state) {
    uint64_t score = 0;
    int maxTile = 0;

    // Find the maximum tile
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            maxTile = std::max(maxTile, Board::getTileAt(state, row, col));
        }
    }

    // Check corners
    int corners[] = {
        Board::getTileAt(state, 0, 0),
        Board::getTileAt(state, 0, 3),
        Board::getTileAt(state, 3, 0),
        Board::getTileAt(state, 3, 3)
    };

    // Reward maximum tile in corner
    for (int corner : corners) {
        if (corner == maxTile) {
            score += 400;  // High reward for keeping max tile in corner
        }
    }

    // Additional reward for maintaining a gradient from the max corner
    if (corners[0] == maxTile) {  // Top-left strategy
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                int tile = Board::getTileAt(state, row, col);
                score += tile * (16 - (row + col));  // Gradient weight
            }
        }
    }

    return score;
}