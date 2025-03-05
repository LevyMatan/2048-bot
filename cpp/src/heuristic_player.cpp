#include "player.hpp"
#include "board.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Helper function to get tile value at position
static int getTileValue(uint64_t state, int row, int col) {
    int shift = ((row * 4 + col) * 4);
    return (state >> shift) & 0xF;
}

std::tuple<Action, uint64_t, int> HeuristicPlayer::chooseAction(uint64_t state) {
    auto validActions = Board::getValidMoveActionsWithScores(state);
    if (validActions.empty()) {
        return {Action::INVALID, state, 0};
    }

    Action bestAction = Action::INVALID;
    uint64_t bestScore = 0;
    uint64_t bestState = state;
    int bestMoveScore = 0;

    for (const auto& [action, nextState, moveScore] : validActions) {
        uint64_t score = evaluatePosition(nextState) + moveScore;
        if (score > bestScore) {
            bestScore = score;
            bestAction = action;
            bestState = nextState;
            bestMoveScore = moveScore;
        }
    }

    return {bestAction, bestState, bestMoveScore};
}

uint64_t HeuristicPlayer::evaluatePosition(uint64_t state) const {
    auto validActions = Board::getValidMoveActions(state);
    if (validActions.empty()) {
        return 0;
    }

    int tileValues[4][4];
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            tileValues[row][col] = getTileValue(state, row, col);
        }
    }

    uint64_t score = 0;
    score += weights.emptyTiles * Board::getEmptyTileCount(state) * 62; // Scale to [0-1000]
    score += weights.monotonicity * evaluateMonotonicity(tileValues);
    score += weights.smoothness * evaluateSmoothness(tileValues);
    score += weights.cornerPlacement * evaluateCornerPlacement(tileValues);
    return score;
}

uint64_t HeuristicPlayer::evaluateMonotonicity(const int tileValues[4][4]) const {
    uint64_t score = 0;

    for (int row = 0; row < 4; ++row) {
        bool increasing = true;
        bool decreasing = true;
        int prev = tileValues[row][0];

        for (int col = 1; col < 4; ++col) {
            int curr = tileValues[row][col];
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
            prev = curr;
        }
        score += (increasing || decreasing) ? 125 : 0; // 1000/8 per line
    }

    for (int col = 0; col < 4; ++col) {
        bool increasing = true;
        bool decreasing = true;
        int prev = tileValues[0][col];

        for (int row = 1; row < 4; ++row) {
            int curr = tileValues[row][col];
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
            prev = curr;
        }
        score += (increasing || decreasing) ? 125 : 0;
    }

    return score;
}

uint64_t HeuristicPlayer::evaluateSmoothness(const int tileValues[4][4]) const {
    uint64_t score = 0;
    uint64_t totalPairs = 0;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            int curr = tileValues[row][col];
            int next = tileValues[row][col + 1];
            if (curr > 0 && next > 0) {
                score += (curr == next) ? 1000 : 500 / (1 + abs(curr - next));
                totalPairs++;
            }
        }
    }

    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 3; ++row) {
            int curr = tileValues[row][col];
            int next = tileValues[row + 1][col];
            if (curr > 0 && next > 0) {
                score += (curr == next) ? 1000 : 500 / (1 + abs(curr - next));
                totalPairs++;
            }
        }
    }

    return totalPairs > 0 ? score / totalPairs : 0;
}

uint64_t HeuristicPlayer::evaluateCornerPlacement(const int tileValues[4][4]) const {
    uint64_t score = 0;
    int maxTile = 0;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            maxTile = std::max(maxTile, tileValues[row][col]);
        }
    }

    int corners[] = {
        tileValues[0][0],
        tileValues[0][3],
        tileValues[3][0],
        tileValues[3][3]
    };

    for (int corner : corners) {
        if (corner == maxTile) {
            score += 250; // 1000/4 per corner
        }
    }

    return score;
}

HeuristicPlayer::HeuristicPlayer(const Weights& w) : weights(w), customName("Heuristic") {}

HeuristicPlayer::HeuristicPlayer(const std::string& weightsFile) : customName("Heuristic-Custom") {
    try {
        weights = loadWeightsFromFile(weightsFile);
        customName = "Heuristic-" + weightsFile;
    } catch (const std::exception& e) {
        std::cerr << "Error loading weights from file: " << e.what() << std::endl;
        std::cerr << "Using default weights instead." << std::endl;
        weights = Weights();
    }
}

HeuristicPlayer::Weights HeuristicPlayer::loadWeightsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open weights file: " + filename);
    }

    std::string line;
    // Skip header line if it exists
    std::getline(file, line);

    // Read the first data line (best weights)
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<double> values;

        // Parse CSV values
        while (std::getline(ss, token, ',')) {
            try {
                values.push_back(std::stod(token));
            } catch (const std::exception&) {
                // Skip non-numeric values
            }
        }

        // Check if we have enough values
        if (values.size() >= 4) {
            return Weights(values[0], values[1], values[2], values[3]);
        }
    }

    throw std::runtime_error("Invalid format in weights file: " + filename);
}

HeuristicPlayer::Weights::Weights(uint64_t e, uint64_t m, uint64_t s, uint64_t c)
    : emptyTiles(e), monotonicity(m), smoothness(s), cornerPlacement(c)
{
    // Ensure weights sum to 1000
    uint64_t total = emptyTiles + monotonicity + smoothness + cornerPlacement;
    if (total != 1000) {
        double scale = 1000.0 / total;
        emptyTiles = static_cast<uint64_t>(emptyTiles * scale);
        monotonicity = static_cast<uint64_t>(monotonicity * scale);
        smoothness = static_cast<uint64_t>(smoothness * scale);
        cornerPlacement = 1000 - emptyTiles - monotonicity - smoothness;
    }
}

std::string HeuristicPlayer::getName() const {
    return customName;
}