#include "expectimax_player.hpp"
#include "board.hpp"
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>
#include <iostream>

ExpectimaxPlayer::ExpectimaxPlayer(const Config& cfg)
    : config(cfg),
      startTime(std::chrono::steady_clock::now()),
      rng(rd()) {
    // Create a custom evaluation function based on the selected evaluation name
    Evaluation::EvalParams params;

    params = Evaluation::getPresetParams(cfg.evalName);

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

    // Initialize best move with the first valid move to ensure we always have something to return
    Action bestAction = std::get<0>(validMoves[0]);
    uint64_t bestState = std::get<1>(validMoves[0]);
    int bestScore = std::get<2>(validMoves[0]);
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
            // Log timeout for debugging
            std::cerr << "Expectimax search timed out after "
                     << std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count()
                     << " seconds" << std::endl;
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

    // Get the current board state to analyze high-value tile positions
    uint8_t board[4][4];
    Evaluation::unpackState(state, board);
    uint8_t maxTile = 0;
    int maxRow = -1, maxCol = -1;

    // Find max tile and its position
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (board[r][c] > maxTile) {
                maxTile = board[r][c];
                maxRow = r;
                maxCol = c;
            }
        }
    }

    // For endgame (few empty tiles), consider all possible tiles for more accurate evaluation
    if (emptyTiles.size() <= 4 && static_cast<int>(emptyTiles.size()) <= config.chanceCovering) {
        // For critical endgame situations, evaluate ALL possible new tile placements
        double totalWeight = 0;

        for (const auto& [row, col] : emptyTiles) {
            // 90% chance of 2 (value 1), 10% chance of 4 (value 2)
            uint64_t newState2 = Board::setTile(state, row, col, 1);
            uint64_t newState4 = Board::setTile(state, row, col, 2);

            uint64_t score2 = maxNode(newState2, depth - 1);
            uint64_t score4 = maxNode(newState4, depth - 1);

            totalScore += 9 * score2;
            totalScore += 1 * score4;

            totalWeight += 10; // 9+1 for each empty tile

            if (shouldTimeOut()) break;
        }

        return totalWeight > 0 ? totalScore / totalWeight : evalFn(state);
    } else {
        // For mid-game situations, use intelligent sampling with position analysis
        // Score tiles based on their position relative to the max tile
        std::vector<std::tuple<int, int, int>> scoredTiles;

        // Analyze the current board structure
        bool maxTileInCorner = (maxRow == 0 || maxRow == 3) && (maxCol == 0 || maxCol == 3);
        bool buildingHorizontal = false;
        bool buildingVertical = false;

        // Detect pattern (horizontal or vertical)
        if (maxTileInCorner) {
            // Check horizontal pattern from the max tile
            int horizontalCount = 0;
            int verticalCount = 0;

            // Check row of max tile
            for (int c = 0; c < 4; c++) {
                if (board[maxRow][c] >= maxTile - 2 && board[maxRow][c] > 0) {
                    horizontalCount++;
                }
            }

            // Check column of max tile
            for (int r = 0; r < 4; r++) {
                if (board[r][maxCol] >= maxTile - 2 && board[r][maxCol] > 0) {
                    verticalCount++;
                }
            }

            buildingHorizontal = horizontalCount > verticalCount;
            buildingVertical = !buildingHorizontal;
        }

        for (const auto& [row, col] : emptyTiles) {
            int posScore = 0;

            // Base positional scoring
            if ((row == 0 || row == 3) && (col == 0 || col == 3)) {
                posScore = 3; // Corner
            } else if (row == 0 || row == 3 || col == 0 || col == 3) {
                posScore = 2; // Edge
            } else {
                posScore = 1; // Interior
            }

            // If we have a high tile in a corner, adjust scores based on pattern
            if (maxTileInCorner && maxTile >= 10) { // 1024 or higher
                if (buildingHorizontal) {
                    // Prefer same row as max tile
                    if (row == maxRow) {
                        posScore += 2;
                    }
                } else if (buildingVertical) {
                    // Prefer same column as max tile
                    if (col == maxCol) {
                        posScore += 2;
                    }
                }

                // Heavily penalize positions next to max tile that would block merges
                bool isAdjacent = (std::abs(row - maxRow) + std::abs(col - maxCol) == 1);
                if (isAdjacent) {
                    posScore -= 1;
                }
            }

            scoredTiles.push_back({row, col, posScore});
        }

        // Sort by position score (higher first)
        std::sort(scoredTiles.begin(), scoredTiles.end(),
                 [](const auto& a, const auto& b) { return std::get<2>(a) > std::get<2>(b); });

        // Then shuffle within each priority group (tiles with same score)
        auto it = scoredTiles.begin();
        while (it != scoredTiles.end()) {
            int currentScore = std::get<2>(*it);
            auto nextGroup = std::find_if(it, scoredTiles.end(),
                                        [currentScore](const auto& item) {
                                            return std::get<2>(item) != currentScore;
                                        });
            std::shuffle(it, nextGroup, rng);
            it = nextGroup;
        }

        int processedSamples = 0;
        double totalWeight = 0;

        // Process the sorted tiles up to numSamples
        for (int i = 0; i < numSamples && i < static_cast<int>(scoredTiles.size()) && !shouldTimeOut(); ++i) {
            auto [row, col, _] = scoredTiles[i];

            // 90% chance of 2 (value 1), 10% chance of 4 (value 2)
            uint64_t newState2 = Board::setTile(state, row, col, 1);
            uint64_t newState4 = Board::setTile(state, row, col, 2);

            totalScore += 9 * maxNode(newState2, depth - 1);
            totalScore += 1 * maxNode(newState4, depth - 1);

            processedSamples++;
            totalWeight += 10; // 9+1
        }

        return processedSamples > 0 ? totalScore / totalWeight : evalFn(state);
    }
}

uint64_t ExpectimaxPlayer::maxNode(uint64_t state, int depth) {
    if (depth <= 0 || shouldTimeOut()) {
        return evalFn(state);
    }

    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return evalFn(state);
    }

    // Sort moves by immediate score to try more promising moves first (move ordering)
    std::sort(validMoves.begin(), validMoves.end(),
             [](const auto& a, const auto& b) {
                 return std::get<2>(a) > std::get<2>(b);
             });

    uint64_t bestValue = 0;

    // Prioritize moves that maintain a good board structure
    // First, check if we have high-value tiles in corners
    bool hasCornerHighTile = false;
    uint8_t board[4][4];
    Evaluation::unpackState(state, board);

    // Identify the highest tile
    uint8_t maxTile = 0;
    int maxRow = 0, maxCol = 0;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (board[r][c] > maxTile) {
                maxTile = board[r][c];
                maxRow = r;
                maxCol = c;
            }
        }
    }

    // Check if highest tile is in a corner
    if ((maxRow == 0 || maxRow == 3) && (maxCol == 0 || maxCol == 3)) {
        hasCornerHighTile = true;
    }

    for (const auto& [action, newState, score] : validMoves) {
        // Special case: if we have a high tile in the corner, heavily penalize moves that disturb it
        // unless we're at a low depth where we don't have much choice
        if (hasCornerHighTile && depth > 2 && maxTile >= 10) { // 2^10 = 1024
            uint8_t newBoard[4][4];
            Evaluation::unpackState(newState, newBoard);

            // If highest tile moved from corner, penalize this move path
            if (newBoard[maxRow][maxCol] != maxTile) {
                // Check if we're in an emergency situation (few moves available)
                if (validMoves.size() > 1) {
                    continue; // Skip this move if we have alternatives
                }
                // Otherwise, evaluate but with penalty (we'll use this move only if we must)
            }
        }

        uint64_t value = chanceNode(newState, depth - 1);
        bestValue = std::max(bestValue, value);

        // Early cutoff: if this move is clearly better than others, no need to evaluate further
        // moves deeply (saving computation)
        if (depth >= 3 && value > bestValue * 1.5 && !shouldTimeOut()) {
            return value; // Early return for clearly dominant move
        }

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
        return config.depth + 4; // Maximum depth for approaching 16K
    } else if (maxTile >= 13) { // 8192
        return config.depth + 3; // Very high depth for approaching 8K
    } else if (maxTile >= 12) { // 4096
        return config.depth + 2; // High depth
    }

    // Depth based on empty tiles and high-value tiles
    if (emptyCount <= 2) {
        return config.depth + 3; // Very critical, search deeper
    } else if (emptyCount <= 4) {
        return config.depth + 2;
    } else if (emptyCount <= 6) {
        return config.depth + 1;
    } else if (emptyCount >= 14) {
        return std::max(2, config.depth - 1); // Board is relatively empty, but never go below depth 2
    }

    // If we have multiple high-value tiles, increase depth slightly to make better decisions
    if (highValueTiles >= 2) {
        return config.depth + 1;
    }

    return config.depth;
}