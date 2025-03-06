#include "expectimax_player.hpp"
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>

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
    uint64_t bestScore = 0;
    int bestMoveScore = 0;

    for (const auto& [action, nextState, moveScore] : validMoves) {
        uint64_t score = chanceNode(nextState, searchDepth - 1);
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

uint64_t ExpectimaxPlayer::expectimax(uint64_t state, int depth, bool isMax) {
    if (depth == 0 || shouldTimeOut()) {
        return evalFn(state);
    }
    return isMax ? maxNode(state, depth) : chanceNode(state, depth);
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
    
    // Create a local random number generator
    static std::random_device rd;
    static std::mt19937 rng(rd());
    
    // Use a larger multiplier to preserve precision in integer division
    const uint64_t PRECISION_MULTIPLIER = 1000;
    uint64_t totalWeight = 0;
    
    for (int i = 0; i < numSamples && !shouldTimeOut(); ++i) {
        // Randomly select an empty tile
        std::uniform_int_distribution<int> dist(0, emptyTiles.size() - 1);
        int randomIndex = dist(rng);
        auto [row, col] = emptyTiles[randomIndex];

        // 90% chance of 2 (value 1), 10% chance of 4 (value 2)
        uint64_t newState2 = Board::setTile(state, row, col, 1);
        uint64_t newState4 = Board::setTile(state, row, col, 2);

        // Scale the weights by the precision multiplier
        uint64_t weight2 = 9 * PRECISION_MULTIPLIER;
        uint64_t weight4 = 1 * PRECISION_MULTIPLIER;
        
        totalScore += weight2 * maxNode(newState2, depth - 1);
        totalScore += weight4 * maxNode(newState4, depth - 1);
        totalWeight += weight2 + weight4;
    }

    // Integer division that preserves more precision
    return totalScore / totalWeight;
}

uint64_t ExpectimaxPlayer::maxNode(uint64_t state, int depth) {
    if (depth <= 0 || shouldTimeOut()) {
        return (evalFn(state));
    }

    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return evalFn(state);
    }

    uint64_t bestScore = 0;

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
    
    // Empty tiles are extremely valuable - each is worth more than any single tile match
    score += static_cast<uint64_t>(Board::getEmptyTileCount(state)) * 10000;
    
    // Track the maximum tile value for scaling
    uint64_t maxTileValue = 0;
    
    // Calculate tile values and find max
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int tileNum = Board::getTileAt(state, row, col);
            if (tileNum > 0) {
                // Get actual value (2^tileNum)
                uint64_t tileValue = 1ULL << tileNum;
                maxTileValue = std::max(maxTileValue, tileValue);
            }
        }
    }
    
    // Add points for adjacent matching tiles (smoothness)
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            int curr = Board::getTileAt(state, row, col);
            int next = Board::getTileAt(state, row, col + 1);
            
            // Reward adjacent tiles of same value
            if (curr > 0 && next > 0) {
                if (curr == next) {
                    // The higher the tile value, the more valuable the match
                    score += static_cast<uint64_t>(1 << curr) * 500;
                }
            }
        }
    }
    
    // Check vertical matches too
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 3; ++row) {
            int curr = Board::getTileAt(state, row, col);
            int next = Board::getTileAt(state, row + 1, col);
            
            if (curr > 0 && next > 0) {
                if (curr == next) {
                    score += static_cast<uint64_t>(1 << curr) * 500;
                }
            }
        }
    }
    
    // Calculate board score based on tile values
    uint64_t boardScore = 0;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int tileNum = Board::getTileAt(state, row, col);
            if (tileNum > 0) {
                // Each tile contributes its value to the score
                boardScore += (1ULL << tileNum);
            }
        }
    }
    score += boardScore * 10;
    
    return score;
}

uint64_t ExpectimaxPlayer::monotonicEvaluation(uint64_t state) {
    uint64_t score = 0;
    const uint64_t MONOTONICITY_WEIGHT = 20000;
    const uint64_t EMPTY_TILE_WEIGHT = 10000;
    const uint64_t MERGE_WEIGHT = 1000;
    
    // Reward empty tiles
    score += Board::getEmptyTileCount(state) * EMPTY_TILE_WEIGHT;
    
    // Monotonicity score - measures how tiles increase/decrease across rows and columns
    for (int row = 0; row < 4; ++row) {
        // Check monotonicity for each row (horizontal)
        uint64_t rowMonoScore = 0;
        bool increasing = true;
        bool decreasing = true;
        
        for (int col = 1; col < 4; ++col) {
            int prev = Board::getTileAt(state, row, col-1);
            int curr = Board::getTileAt(state, row, col);
            
            if (prev > 0 && curr > 0) {
                if (curr < prev) increasing = false;
                if (curr > prev) decreasing = false;
                
                // Add penalty for non-monotonic differences
                if (!increasing && !decreasing) {
                    // Different weights for increasing vs decreasing patterns
                    rowMonoScore = 0;
                    break;
                }
            }
        }
        
        if (increasing || decreasing) {
            // Give extra weight to rows with higher value tiles
            uint64_t rowMaxTile = 0;
            for (int col = 0; col < 4; ++col) {
                rowMaxTile = std::max(rowMaxTile, 
                                     static_cast<uint64_t>(1 << Board::getTileAt(state, row, col)));
            }
            rowMonoScore = MONOTONICITY_WEIGHT + (rowMaxTile / 10);
        }
        
        score += rowMonoScore;
    }
    
    // Check vertical monotonicity
    for (int col = 0; col < 4; ++col) {
        uint64_t colMonoScore = 0;
        bool increasing = true;
        bool decreasing = true;
        
        for (int row = 1; row < 4; ++row) {
            int prev = Board::getTileAt(state, row-1, col);
            int curr = Board::getTileAt(state, row, col);
            
            if (prev > 0 && curr > 0) {
                if (curr < prev) increasing = false;
                if (curr > prev) decreasing = false;
                
                if (!increasing && !decreasing) {
                    colMonoScore = 0;
                    break;
                }
            }
        }
        
        if (increasing || decreasing) {
            uint64_t colMaxTile = 0;
            for (int row = 0; row < 4; ++row) {
                colMaxTile = std::max(colMaxTile, 
                                     static_cast<uint64_t>(1 << Board::getTileAt(state, row, col)));
            }
            colMonoScore = MONOTONICITY_WEIGHT + (colMaxTile / 10);
        }
        
        score += colMonoScore;
    }
    
    // Reward potential merges
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            int curr = Board::getTileAt(state, row, col);
            int next = Board::getTileAt(state, row, col + 1);
            if (curr > 0 && curr == next) {
                score += MERGE_WEIGHT * (1 << curr);
            }
        }
    }
    
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 3; ++row) {
            int curr = Board::getTileAt(state, row, col);
            int next = Board::getTileAt(state, row + 1, col);
            if (curr > 0 && curr == next) {
                score += MERGE_WEIGHT * (1 << curr);
            }
        }
    }
    
    return score;
}

uint64_t ExpectimaxPlayer::cornerEvaluation(uint64_t state) {
    uint64_t score = 0;
    int maxTile = 0;
    int maxTileRow = -1;
    int maxTileCol = -1;
    const uint64_t CORNER_WEIGHT = 50000;
    const uint64_t GRADIENT_WEIGHT = 1000;
    const uint64_t EMPTY_TILE_WEIGHT = 10000;
    
    // Find the maximum tile and its position
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int tileValue = Board::getTileAt(state, row, col);
            if (tileValue > maxTile) {
                maxTile = tileValue;
                maxTileRow = row;
                maxTileCol = col;
            }
        }
    }
    
    // Empty tiles are valuable
    score += Board::getEmptyTileCount(state) * EMPTY_TILE_WEIGHT;
    
    // Check corners
    int corners[4][2] = {
        {0, 0}, {0, 3}, {3, 0}, {3, 3}
    };
    
    bool maxInCorner = false;
    int bestCornerRow = 0;
    int bestCornerCol = 0;
    
    // Reward maximum tile in corner
    for (int i = 0; i < 4; ++i) {
        int cornerRow = corners[i][0];
        int cornerCol = corners[i][1];
        int cornerTile = Board::getTileAt(state, cornerRow, cornerCol);
        
        if (cornerTile == maxTile) {
            score += CORNER_WEIGHT * (1 << maxTile);
            maxInCorner = true;
            bestCornerRow = cornerRow;
            bestCornerCol = cornerCol;
            break;
        }
    }
    
    // If max tile is not in corner, give partial points based on how close it is to a corner
    if (!maxInCorner && maxTile > 0) {
        // Find closest corner
        int minDistance = 6; // max Manhattan distance on 4x4 grid is 6
        
        for (int i = 0; i < 4; ++i) {
            int cornerRow = corners[i][0];
            int cornerCol = corners[i][1];
            int distance = std::abs(maxTileRow - cornerRow) + std::abs(maxTileCol - cornerCol);
            
            if (distance < minDistance) {
                minDistance = distance;
                bestCornerRow = cornerRow;
                bestCornerCol = cornerCol;
            }
        }
        
        // Give partial points for being close to a corner
        score += CORNER_WEIGHT * (1 << maxTile) / (minDistance + 1);
    }
    
    // Additional reward for maintaining a gradient from the max corner
    if (maxInCorner) {
        // Calculate gradient direction
        int rowDir = (bestCornerRow == 0) ? 1 : -1;
        int colDir = (bestCornerCol == 0) ? 1 : -1;
        
        // Check gradient
        uint64_t gradientScore = 0;
        
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                int tile = Board::getTileAt(state, row, col);
                if (tile > 0) {
                    // Calculate gradient weight based on distance from corner
                    int distFromCorner = std::abs(row - bestCornerRow) + std::abs(col - bestCornerCol);
                    
                    // In a good gradient, tiles decrease as distance increases
                    if (distFromCorner > 0) {
                        int expectedDecrease = distFromCorner * 2;
                        int actualValue = maxTile - tile;
                        
                        // Perfect gradient: actualValue == expectedDecrease
                        // Bad gradient: actualValue < 0 (means higher values further from corner)
                        if (actualValue >= 0 && actualValue <= expectedDecrease) {
                            gradientScore += (1 << tile) * (expectedDecrease - actualValue);
                        }
                    }
                }
            }
        }
        
        score += (gradientScore * GRADIENT_WEIGHT) / 100;
    }
    
    return score;
}