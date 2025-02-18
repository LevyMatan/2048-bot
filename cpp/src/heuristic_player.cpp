#include "player.hpp"
#include "game.hpp"
#include <cmath>

HeuristicPlayer::HeuristicPlayer(const Weights& w) : weights(w) {}

std::pair<int, uint64_t> HeuristicPlayer::chooseAction(uint64_t state) {
    auto validActions = Board::getValidMoveActions(state);
    if (validActions.empty()) {
        return {-1, state};
    }

    double bestScore = -std::numeric_limits<double>::infinity();
    std::pair<int, uint64_t> bestAction;

    for (const auto& [action, nextState] : validActions) {
        double score = evaluatePosition(nextState);
        if (score > bestScore) {
            bestScore = score;
            bestAction = {action, nextState};
        }
    }

    return bestAction;
}

std::string HeuristicPlayer::getName() const {
    return "Heuristic Player";
}

double HeuristicPlayer::evaluatePosition(uint64_t state) const {
    double score = 0.0;
    score += weights.emptyTiles * Board::getEmptyTiles(state).size();
    score += weights.monotonicity * evaluateMonotonicity(state);
    score += weights.smoothness * evaluateSmoothness(state);
    score += weights.cornerPlacement * evaluateCornerPlacement(state);
    return score;
}

double HeuristicPlayer::evaluateMonotonicity(uint64_t state) const {
    double score = 0.0;
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
    return score / 8.0;
}

double HeuristicPlayer::evaluateSmoothness(uint64_t state) const {
    double score = 0.0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int current = (state >> ((15-i*4-j) * 4)) & 0xF;
            if (current == 0) continue;

            if (j < 3) {
                int right = (state >> ((15-i*4-j-1) * 4)) & 0xF;
                if (right != 0) {
                    score += 1.0 / (1.0 + std::abs(current - right));
                }
            }
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

double HeuristicPlayer::evaluateCornerPlacement(uint64_t state) const {
    double score = 0.0;
    int maxValue = 0;
    int maxPos = -1;

    for (int i = 0; i < 16; ++i) {
        int value = (state >> (i * 4)) & 0xF;
        if (value > maxValue) {
            maxValue = value;
            maxPos = i;
        }
    }

    if (maxPos == 0 || maxPos == 3 || maxPos == 12 || maxPos == 15) {
        score += 2.0;
    }

    return score;
}