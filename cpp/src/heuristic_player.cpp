#include "player.hpp"
#include "board.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

std::pair<int, uint64_t> HeuristicPlayer::chooseAction(uint64_t state) {
    auto validActions = Board::getValidMoveActions(state);
    if (validActions.empty()) {
        return {-1, state};
    }

    int bestAction = -1;
    double bestScore = -std::numeric_limits<double>::infinity();
    uint64_t bestState = state;

    for (const auto& [action, nextState] : validActions) {
        double score = evaluatePosition(nextState);
        if (score > bestScore) {
            bestScore = score;
            bestAction = static_cast<int>(action);
            bestState = nextState;
        }
    }

    return {bestAction, bestState};
}

double HeuristicPlayer::evaluatePosition(uint64_t state) const {
    auto validActions = Board::getValidMoveActions(state);
    if (validActions.empty()) {
        return -std::numeric_limits<double>::infinity();
    }

    double score = 0.0;
    score += weights.emptyTiles * Board::getEmptyTiles(state).size();
    score += weights.monotonicity * evaluateMonotonicity(state);
    score += weights.smoothness * evaluateSmoothness(state);
    score += weights.cornerPlacement * evaluateCornerPlacement(state);
    return score;
}

// Helper function to get tile value at position
static int getTileValue(uint64_t state, int row, int col) {
    int shift = (15 - (row * 4 + col)) * 4;
    return (state >> shift) & 0xF;
}

double HeuristicPlayer::evaluateMonotonicity(uint64_t state) const {
    double score = 0.0;
    
    // Check rows
    for (int row = 0; row < 4; ++row) {
        bool increasing = true;
        bool decreasing = true;
        int prev = getTileValue(state, row, 0);
        
        for (int col = 1; col < 4; ++col) {
            int curr = getTileValue(state, row, col);
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
            prev = curr;
        }
        score += increasing || decreasing ? 1.0 : 0.0;
    }
    
    // Check columns
    for (int col = 0; col < 4; ++col) {
        bool increasing = true;
        bool decreasing = true;
        int prev = getTileValue(state, 0, col);
        
        for (int row = 1; row < 4; ++row) {
            int curr = getTileValue(state, row, col);
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
            prev = curr;
        }
        score += increasing || decreasing ? 1.0 : 0.0;
    }
    
    return score / 8.0;  // Normalize to [0, 1]
}

double HeuristicPlayer::evaluateSmoothness(uint64_t state) const {
    double smoothness = 0.0;
    int totalTiles = 0;
    
    // Check horizontal smoothness
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            int curr = getTileValue(state, row, col);
            int next = getTileValue(state, row, col + 1);
            if (curr > 0 && next > 0) {
                smoothness += 1.0 / (1.0 + std::abs(curr - next));
                totalTiles++;
            }
        }
    }
    
    // Check vertical smoothness
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 3; ++row) {
            int curr = getTileValue(state, row, col);
            int next = getTileValue(state, row + 1, col);
            if (curr > 0 && next > 0) {
                smoothness += 1.0 / (1.0 + std::abs(curr - next));
                totalTiles++;
            }
        }
    }
    
    return totalTiles > 0 ? smoothness / totalTiles : 0.0;
}

double HeuristicPlayer::evaluateCornerPlacement(uint64_t state) const {
    double score = 0.0;
    int maxTile = 0;
    
    // Find the maximum tile
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            maxTile = std::max(maxTile, getTileValue(state, row, col));
        }
    }
    
    // Check corners
    int corners[] = {
        getTileValue(state, 0, 0),
        getTileValue(state, 0, 3),
        getTileValue(state, 3, 0),
        getTileValue(state, 3, 3)
    };
    
    for (int corner : corners) {
        if (corner == maxTile) {
            score += 1.0;
        }
    }
    
    return score / 4.0;  // Normalize to [0, 1]
}

HeuristicPlayer::HeuristicPlayer(const Weights& w) : weights(w) {}

std::string HeuristicPlayer::getName() const {
    return "Heuristic";
}