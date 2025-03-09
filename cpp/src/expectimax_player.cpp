#include "players.hpp"
#include "board.hpp"
#include "evaluation.hpp"
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>
#include <iostream>
#include <unordered_map>

ExpectimaxPlayer::ExpectimaxPlayer(const int depth, const int chanceCovering,
    const double timeLimit, const bool adaptive_depth, const Evaluation::EvalParams& params)
    : depthLimit(depth),
      chanceCovering(chanceCovering),
      timeLimit(timeLimit),
      adaptiveDepth(adaptive_depth),
      evalParams(params),
      evaluator(params){

    evaluator = Evaluation::CompositeEvaluator(params);
    evalFn = [this](BoardState state) -> double {
        return this->evaluator.evaluate(state);
    };
}

ChosenActionResult ExpectimaxPlayer::chooseAction(uint64_t state) {
    startTime = std::chrono::steady_clock::now();

    int searchDepth = adaptiveDepth ?
                     getAdaptiveDepth(state) : depthLimit;

    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return {Action::INVALID, state, 0};
    }

    // Initialize best move with the first valid move to ensure we always have something to return
    ChosenActionResult bestAction = validMoves[0];

    double bestValue = 0.0;

    for (const ChosenActionResult& actionResult : validMoves) {
        double value = chanceNode(actionResult.state, searchDepth, 1.0);

        if (value > bestValue) {
            bestValue = value;
            bestAction = actionResult;
        }


        if (shouldTimeOut()) {
            // Log timeout for debugging
            std::cerr << "Expectimax search timed out after "
                     << std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count()
                     << " seconds" << std::endl;
            break;
        }
    }

    return bestAction;
}

double ExpectimaxPlayer::chanceNode(BoardState state, int depth, double prob) {
    if (depth <= 0 || shouldTimeOut() || prob < 0.001) {
        return evalFn(state);
    }

    auto emptyTiles = Board::getEmptyTiles(state);
    if (emptyTiles.empty()) {
        return evalFn(state);
    }

    uint64_t num_open = emptyTiles.size();
    prob /= num_open;

    double res = 0.0;
    BoardState tmp = state;
    BoardState tile_2 = 1;
    while (tile_2) {
        if ((tmp & 0xf) == 0) {
            double value2 = maxNode(state | tile_2, depth - 1, prob * 0.9) * 0.9;
            double value4 = maxNode(state | (tile_2 << 1U), depth - 1, prob * 0.1) * 0.1;
            res += value2;
            res += value4;
        }
        tmp >>= 4;
        tile_2 <<= 4;
    }
    res = res / num_open;
    return res;
}

double ExpectimaxPlayer::maxNode(BoardState state, int depth, double prob) {
    if (depth <= 0 || shouldTimeOut()) {
        return evalFn(state);
    }

    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return evalFn(state);
    }

    double bestValue = std::numeric_limits<double>::lowest();
    for (const auto& [action, newState, score] : validMoves) {
        double value = chanceNode(newState, depth - 1, prob);
        bestValue = std::max(bestValue, value);
    }
    return bestValue;
}

bool ExpectimaxPlayer::shouldTimeOut() const {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration<double>(currentTime - startTime).count();
    return elapsedTime >= timeLimit;
}

int ExpectimaxPlayer::getAdaptiveDepth(uint64_t state) const {
    auto emptyTiles = Board::getEmptyTiles(state);
    int emptyCount = static_cast<int>(emptyTiles.size());

    // Get the maximum tile to adjust depth for endgame strategy
    uint8_t board[4][4];
    Evaluation::unpackState(state, board);
    uint8_t maxTile = Evaluation::findMaxTile(board);

    // Calculate the number of high-value tiles (2048+)
    int highValueTiles = 0;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (board[row][col] >= 11) { // 2^11 = 2048
                highValueTiles++;
            }
        }
    }

    // Super aggressive depth for critical situations
    if (maxTile >= 14) { // 16384 or higher
        return depthLimit + 4; // Maximum depth for approaching 16K
    } else if (maxTile >= 13) { // 8192
        return depthLimit + 3; // Very high depth for approaching 8K
    } else if (maxTile >= 12) { // 4096
        return depthLimit + 2; // High depth
    }

    // Depth based on empty tiles and high-value tiles
    if (emptyCount <= 2) {
        return depthLimit + 3; // Very critical, search deeper
    } else if (emptyCount <= 4) {
        return depthLimit + 2;
    } else if (emptyCount <= 6) {
        return depthLimit + 1;
    } else if (emptyCount >= 14) {
        return std::max(2, depthLimit - 1); // Board is relatively empty, but never go below depth 2
    }

    // If we have multiple high-value tiles, increase depth slightly to make better decisions
    if (highValueTiles >= 2) {
        return depthLimit + 1;
    }

    return depthLimit;
}