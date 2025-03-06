#include "expectimax_player.hpp"
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>

ExpectimaxPlayer::ExpectimaxPlayer(const Config& cfg)
    : config(cfg),
      startTime(std::chrono::steady_clock::now()),
      rng(rd()) {
    // Create a custom evaluation function based on the selected evaluation name
    Evaluation::EvalParams params;
    
    if (cfg.evalName == "corner") {
        params["cornerValue"] = 1000;
    } else if (cfg.evalName == "standard") {
        params["emptyTiles"] = 250;
        params["monotonicity"] = 250;
        params["smoothness"] = 250;
        params["cornerValue"] = 250;
    } else if (cfg.evalName == "merge") {
        params["mergeability"] = 1000;
    } else if (cfg.evalName == "pattern") {
        params["patternMatching"] = 1000;
    } else if (cfg.evalName == "balanced") {
        params["emptyTiles"] = 200;
        params["monotonicity"] = 200;
        params["smoothness"] = 200;
        params["cornerValue"] = 200;
        params["patternMatching"] = 200;
    } else {
        // Default to combined
        params["emptyTiles"] = 250;
        params["monotonicity"] = 250;
        params["smoothness"] = 250;
        params["cornerValue"] = 250;
    }
    
    Evaluation::CompositeEvaluator evaluator(params);
    evalFn = [evaluator](uint64_t state) {
        return evaluator.evaluate(state);
    };
}

ExpectimaxPlayer::ExpectimaxPlayer(const Config& cfg, EvaluationFunction fn)
    : config(cfg),
      evalFn(fn),
      startTime(std::chrono::steady_clock::now()),
      rng(rd()) {}

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
    int bestScore = 0;
    uint64_t bestValue = 0;

    for (const auto& [action, newState, score] : validMoves) {
        uint64_t value = chanceNode(newState, searchDepth);
        
        if (value > bestValue) {
            bestValue = value;
            bestAction = action;
            bestState = newState;
            bestScore = score;
        }
        
        if (shouldTimeOut()) {
            break;
        }
    }

    return {bestAction, bestState, bestScore};
}

uint64_t ExpectimaxPlayer::chanceNode(uint64_t state, int depth) {
    if (depth <= 0 || shouldTimeOut()) {
        return evalFn(state);
    }

    auto emptyTiles = Board::getEmptyTiles(state);
    if (emptyTiles.empty()) {
        return evalFn(state);
    }

    uint64_t totalScore = 0;
    int numSamples = std::min(config.chanceCovering,
                             static_cast<int>(emptyTiles.size()));

    // Use random shuffling for more diverse sampling
    std::shuffle(emptyTiles.begin(), emptyTiles.end(), rng);

    for (int i = 0; i < numSamples && !shouldTimeOut(); ++i) {
        auto [row, col] = emptyTiles[i % emptyTiles.size()];

        // 90% chance of 2 (value 1), 10% chance of 4 (value 2)
        uint64_t newState2 = Board::setTile(state, row, col, 1);
        uint64_t newState4 = Board::setTile(state, row, col, 2);

        totalScore += 9 * maxNode(newState2, depth - 1);
        totalScore += 1 * maxNode(newState4, depth - 1);
    }

    return totalScore / (numSamples * 10); // Divide by total weight (9+1)*numSamples
}

uint64_t ExpectimaxPlayer::maxNode(uint64_t state, int depth) {
    if (depth <= 0 || shouldTimeOut()) {
        return evalFn(state);
    }

    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return evalFn(state);
    }

    uint64_t bestValue = 0;

    for (const auto& [action, newState, score] : validMoves) {
        uint64_t value = chanceNode(newState, depth - 1);
        bestValue = std::max(bestValue, value);
        
        if (shouldTimeOut()) {
            break;
        }
    }

    return bestValue;
}

bool ExpectimaxPlayer::shouldTimeOut() const {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration<double>(currentTime - startTime).count();
    return elapsedTime >= config.timeLimit;
}

int ExpectimaxPlayer::getAdaptiveDepth(uint64_t state) const {
    auto emptyTiles = Board::getEmptyTiles(state);
    int emptyCount = static_cast<int>(emptyTiles.size());
    
    // Adaptive depth based on empty tile count
    if (emptyCount <= 4) {
        return config.depth + 2;
    } else if (emptyCount <= 6) {
        return config.depth + 1;
    } else if (emptyCount >= 12) {
        return std::max(1, config.depth - 1);
    } else {
        return config.depth;
    }
} 