#include "evaluation.hpp"
#include "board.hpp"
#include <algorithm>
#include <cmath>
#include <array>
#include <unordered_map>

namespace Evaluation {

//------------------------------------------------------
// Helper functions
//------------------------------------------------------

// Helper function to unpack state into a 2D board
void unpackState(uint64_t state, uint8_t board[4][4]) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            board[row][col] = static_cast<uint8_t>(
                (state >> ((row * 4 + col) * 4)) & 0xF
            );
        }
    }
}

// Utility function to calculate the score from the board state
uint64_t calculateScore(const uint8_t board[4][4]) {
    uint64_t score = 0;
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            uint8_t tileValue = board[row][col];
            if (tileValue > 1) { // Only score tiles > 2 (log base 2)
                score += (1ULL << tileValue);
            }
        }
    }
    
    return score;
}

// Find the maximum tile value
uint8_t findMaxTile(const uint8_t board[4][4]) {
    uint8_t maxTile = 0;
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            maxTile = std::max(maxTile, board[row][col]);
        }
    }
    
    return maxTile;
}

//------------------------------------------------------
// Core evaluation functions
//------------------------------------------------------

// Count empty tiles in the board (normalized to 0-1000)
uint64_t emptyTiles(const uint8_t board[4][4]) {
    uint64_t count = 0;
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (board[row][col] == 0) {
                count++;
            }
        }
    }
    
    // Normalize: Max empty tiles is 16, so multiply by 1000/16 = 62.5
    return count * 1000 / 16;
}

// MONOTONICITY: Evaluates how well the tiles are arranged in increasing/decreasing order
uint64_t monotonicity(const uint8_t board[4][4]) {
    uint64_t score = 0;

    for (int row = 0; row < 4; ++row) {
        bool increasing = true;
        bool decreasing = true;
        int prev = board[row][0];

        for (int col = 1; col < 4; ++col) {
            int curr = board[row][col];
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
            prev = curr;
        }
        score += (increasing || decreasing) ? 125 : 0; // 1000/8 per line
    }

    for (int col = 0; col < 4; ++col) {
        bool increasing = true;
        bool decreasing = true;
        int prev = board[0][col];

        for (int row = 1; row < 4; ++row) {
            int curr = board[row][col];
            if (curr < prev) increasing = false;
            if (curr > prev) decreasing = false;
            prev = curr;
        }
        score += (increasing || decreasing) ? 125 : 0;
    }

    return score;
}

// MERGEABILITY: Evaluates the potential for merging adjacent tiles (normalized to 0-1000)
uint64_t mergeability(const uint8_t board[4][4]) {
    uint64_t score = 0;
    uint64_t maxScore = 0;
    uint8_t maxTile = findMaxTile(board);
    
    // Theoretical max score if all tiles have the same value
    // In a 4x4 grid, there are 24 adjacent pairs:
    // - 12 horizontal pairs (3 per row × 4 rows)
    // - 12 vertical pairs (3 per column × 4 columns)
    // But at most all 16 tiles having the same value gives 24 merge possibilities
    if (maxTile > 1) {
        maxScore = 24 * (1ULL << maxTile) * 2;
    } else {
        // If no significant tiles, set a reasonable minimum
        maxScore = 2048; 
    }
    
    // Check horizontal merge potential
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            if (board[row][col] > 0 && board[row][col] == board[row][col+1]) {
                // Higher value tiles are worth more to merge
                score += (1ULL << board[row][col]) * 2;
            }
        }
    }
    
    // Check vertical merge potential
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 3; row++) {
            if (board[row][col] > 0 && board[row][col] == board[row+1][col]) {
                // Higher value tiles are worth more to merge
                score += (1ULL << board[row][col]) * 2;
            }
        }
    }
    
    // Normalize to 0-1000
    return std::min(1000ULL, (score * 1000) / maxScore);
}

// SMOOTHNESS: Evaluates how smooth/gradual the transitions between adjacent tiles are
uint64_t smoothness(const uint8_t board[4][4]) {
    uint64_t score = 0;
    uint64_t totalPairs = 0;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            int curr = board[row][col];
            int next = board[row][col + 1];
            if (curr > 0 && next > 0) {
                score += (curr == next) ? 1000 : 500 / (1 + abs(curr - next));
                totalPairs++;
            }
        }
    }

    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 3; ++row) {
            int curr = board[row][col];
            int next = board[row + 1][col];
            if (curr > 0 && next > 0) {
                score += (curr == next) ? 1000 : 500 / (1 + abs(curr - next));
                totalPairs++;
            }
        }
    }

    return totalPairs > 0 ? score / totalPairs : 0;
}

// CORNER VALUE: Evaluates how well the highest values are positioned in corners
uint64_t cornerValue(const uint8_t board[4][4]) {
    uint64_t score = 0;
    uint8_t maxTile = findMaxTile(board);

    uint8_t corners[] = {
        board[0][0],
        board[0][3],
        board[3][0],
        board[3][3]
    };

    for (uint8_t corner : corners) {
        if (corner == maxTile) {
            score += 250; // 1000/4 per corner
        }
    }

    return score;
}

// PATTERN MATCHING: Evaluates how well the board matches desired patterns (normalized to 0-1000)
uint64_t patternMatching(const uint8_t board[4][4]) {
    // Snake pattern weights
    const uint64_t snakeWeights[4][4] = {
        {15, 14, 13, 12},
        {8,  9,  10, 11},
        {7,  6,  5,  4},
        {0,  1,  2,  3}
    };
    
    uint64_t score = 0;
    uint64_t maxScore = 0;
    uint8_t maxTile = findMaxTile(board);
    
    // Calculate theoretical max score
    if (maxTile > 1) {
        // Calculate optimal pattern score if tiles were arranged optimally
        // Highest possible score would be if the 16 highest possible tiles
        // were arranged in descending order according to the weight pattern
        uint64_t maxTileValue = 1ULL << maxTile;
        uint64_t sumOfWeights = 0;
        
        // Sum the weights of the snake pattern
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                sumOfWeights += snakeWeights[row][col];
            }
        }
        
        // Maximum possible score is the max tile value times the sum of weights
        maxScore = maxTileValue * sumOfWeights;
    } else {
        // If no significant tiles, set a reasonable minimum
        maxScore = 2048;
    }
    
    // Calculate pattern score
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (board[row][col] > 0) {
                // Each tile contributes its value multiplied by its weight in the pattern
                score += (1ULL << board[row][col]) * snakeWeights[row][col];
            }
        }
    }
    
    // Normalize to 0-1000
    return std::min(1000ULL, (score * 1000) / maxScore);
}

SimpleEvalFunc getNamedEvaluation(const std::string& name) {
    if (name == "emptyTiles") return emptyTiles;
    if (name == "monotonicity") return monotonicity;
    if (name == "mergeability") return mergeability;
    if (name == "smoothness") return smoothness;
    if (name == "cornerValue") return cornerValue;
    if (name == "patternMatching") return patternMatching;
    return nullptr;
}

// Function to get preset parameter configurations
EvalParams getPresetParams(const std::string& name) {
    EvalParams params;
    
    if (name == "standard" || name == "combined") {
        params["emptyTiles"] = 250;
        params["monotonicity"] = 250;
        params["smoothness"] = 250;
        params["cornerValue"] = 250;
        return params;
    }
    else if (name == "corner") {
        params["cornerValue"] = 1000;
        return params;
    }
    else if (name == "merge") {
        params["mergeability"] = 1000;
        return params;
    }
    else if (name == "pattern") {
        params["patternMatching"] = 1000;
        return params;
    }
    else if (name == "balanced") {
        params["emptyTiles"] = 200;
        params["monotonicity"] = 200;
        params["smoothness"] = 200;
        params["cornerValue"] = 200;
        params["patternMatching"] = 200;
        return params;
    }
    
    // If no match, return standard parameters
    params["emptyTiles"] = 250;
    params["monotonicity"] = 250;
    params["smoothness"] = 250;
    params["cornerValue"] = 250;
    return params;
}

//------------------------------------------------------
// CompositeEvaluator implementation
//------------------------------------------------------

CompositeEvaluator::CompositeEvaluator(EvalParams params) {
    for (const auto& [name, weight] : params) {
        addComponent(getNamedEvaluation(name), weight, name);
    }
    if (params.empty()) {
        addComponent(emptyTiles, 1000, "emptyTiles");
    }
}

void CompositeEvaluator::addComponent(SimpleEvalFunc func, uint64_t weight, const std::string& name) {
    components.emplace_back(func, weight, name);
    componentIndices[name] = components.size() - 1;
}

uint64_t CompositeEvaluator::evaluate(uint64_t state) const {
    // Unpack the state first
    uint8_t board[4][4];
    unpackState(state, board);
    
    // Apply each component
    uint64_t totalScore = 0;
    
    for (const auto& component : components) {
        totalScore += component.function(board) * component.weight;
    }
    
    return totalScore;
}

void CompositeEvaluator::setWeight(const std::string& name, uint64_t weight) {
    auto it = componentIndices.find(name);
    if (it != componentIndices.end()) {
        components[it->second].weight = weight;
    }
}

uint64_t CompositeEvaluator::getWeight(const std::string& name) const {
    auto it = componentIndices.find(name);
    if (it != componentIndices.end()) {
        return components[it->second].weight;
    }
    return 0;
}

EvalParams CompositeEvaluator::getParams() const {
    EvalParams params;
    for (const auto& component : components) {
        params[component.name] = component.weight;
    }
    return params;
}

void CompositeEvaluator::setParams(const EvalParams& params) {
    for (const auto& [name, weight] : params) {
        setWeight(name, weight);
    }
}



} // namespace Evaluation 