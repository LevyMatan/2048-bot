#include "evaluation.hpp"
#include "board.hpp"
#include "logger.hpp"
#include <algorithm>
#include <cmath>
#include <array>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>

extern Logger2048::Logger &logger;

namespace Evaluation {

//------------------------------------------------------
// Helper functions
//------------------------------------------------------

// Helper function to unpack state into a 2D board
void unpackState(BoardState state, uint8_t board[4][4]) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            board[row][col] = static_cast<uint8_t>(
                (state >> ((row * 4 + col) * 4)) & 0xF
            );
        }
    }
}

// Utility function to calculate the score from the board state
double calculateScore(const uint8_t board[4][4]) {
    uint64_t score = 0;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            uint8_t tileValue = board[row][col];
            if (tileValue > 1) { // Only score tiles > 2 (log base 2)
                score += (1ULL << tileValue);
            }
        }
    }

    return static_cast<double>(score);
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

double coreScore(const uint8_t board[4][4]) {
    return calculateScore(board);
}

// Count empty tiles in the board (normalized to 0-1000)
double emptyTiles(const uint8_t board[4][4]) {
    uint64_t emptyCount = 0;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (board[row][col] == 0) {
                emptyCount++;
            }
        }
    }

    // Normalize: Max empty tiles is 16, so multiply by 1000/16 = 62.5
    return std::pow(2, emptyCount);
}

// MONOTONICITY: Evaluates how well the tiles are arranged in increasing/decreasing order
double monotonicity(const uint8_t board[4][4]) {
    double score = 0.0;

    // Check rows and columns for monotonicity
    for (int i = 0; i < 4; ++i) {
        double rowScore = 0.0;
        double colScore = 0.0;

        // Check increasing and decreasing patterns
        for (int j = 0; j < 3; ++j) {
            // Row check
            int rowDiff = board[i][j+1] - board[i][j];
            if (rowDiff >= 0) rowScore += 1.0 / (1.0 + std::abs(rowDiff));

            // Column check
            int colDiff = board[j+1][i] - board[j][i];
            if (colDiff >= 0) colScore += 1.0 / (1.0 + std::abs(colDiff));
        }

        score += std::max(rowScore, 3.0 - rowScore) * 125.0;  // Normalize to 0-125 per line
        score += std::max(colScore, 3.0 - colScore) * 125.0;
    }

    return score;
}

// MERGEABILITY: Evaluates the potential for merging adjacent tiles (normalized to 0-1000)
double mergeability(const uint8_t board[4][4]) {
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
    return std::min(1000.0, (score * 1000.0) / maxScore);
}

// SMOOTHNESS: Evaluates how smooth/gradual the transitions between adjacent tiles are
double smoothness(const uint8_t board[4][4]) {
    double score = 0.0;
    double totalWeight = 0.0;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (board[row][col] == 0) continue;

            // Check right neighbor
            if (col < 3 && board[row][col+1] > 0) {
                int diff = std::abs(board[row][col] - board[row][col+1]);
                double weight = std::pow(2.0, std::max(board[row][col], board[row][col+1]));
                score += weight * (1.0 / (1.0 + diff));
                totalWeight += weight;
            }

            // Check bottom neighbor
            if (row < 3 && board[row+1][col] > 0) {
                int diff = std::abs(board[row][col] - board[row+1][col]);
                double weight = std::pow(2.0, std::max(board[row][col], board[row+1][col]));
                score += weight * (1.0 / (1.0 + diff));
                totalWeight += weight;
            }
        }
    }

    return totalWeight > 0 ? (score / totalWeight) * 1000.0 : 0.0;
}

// CORNER VALUE: Evaluates how well the highest values are positioned in corners
double cornerValue(const uint8_t board[4][4]) {
    double score = 0.0;
    uint8_t maxTile = findMaxTile(board);

    // Corner positions and their weights
    const std::pair<int, int> corners[4] = {{0,0}, {0,3}, {3,0}, {3,3}};

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (board[row][col] == 0) continue;

            // Calculate minimum distance to any corner
            double minDistance = 6.0; // Max possible distance is 6 (3+3)
            for (const auto& [crow, ccol] : corners) {
                double distance = std::abs(row - crow) + std::abs(col - ccol);
                minDistance = std::min(minDistance, distance);
            }

            // Higher value tiles should be closer to corners
            double tileWeight = std::pow(2.0, board[row][col]);
            double distanceScore = (6.0 - minDistance) / 6.0; // Normalize to 0-1
            score += tileWeight * distanceScore;
        }
    }

    // Normalize to 0-1000
    double maxPossibleScore = std::pow(2.0, maxTile) * 4.0; // 4 corners
    return std::min(1000.0, (score * 1000.0) / maxPossibleScore);
}

// PATTERN MATCHING: Evaluates how well the board matches desired patterns (normalized to 0-1000)
double patternMatching(const uint8_t board[4][4]) {
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
    return std::min(1000.0, (score * 1000.0) / maxScore);
}

SimpleEvalFunc getNamedEvaluation(const std::string& name) {
    if (name == "emptyTiles") return emptyTiles;
    if (name == "monotonicity") return monotonicity;
    if (name == "mergeability") return mergeability;
    if (name == "smoothness") return smoothness;
    if (name == "cornerValue") return cornerValue;
    if (name == "patternMatching") return patternMatching;
    if (name == "coreScore") return coreScore;
    return nullptr;
}

// Dictionary of preset evaluation parameters
static const std::unordered_map<std::string, EvalParams> PRESET_PARAMS = {
    {"standard", {{"emptyTiles", 250}, {"monotonicity", 250}, {"smoothness", 250}, {"cornerValue", 250}, {"coreScore", 250}}},
    {"corner", {{"cornerValue", 1000}}},
    {"merge", {{"mergeability", 1000}}},
    {"pattern", {{"patternMatching", 1000}}},
    {"balanced", {{"emptyTiles", 200}, {"monotonicity", 200}, {"smoothness", 200}, {"cornerValue", 200}, {"patternMatching", 200}}},
    {"empty", {{"emptyTiles", 1000}}},
    {"best", {{"emptyTiles", 427}, {"monotonicity", 12}, {"smoothness", 29}, {"cornerValue", 67}, {"patternMatching", 186}}},
};

// Function to get preset parameter configurations
EvalParams getPresetParams(const std::string& name) {
    auto it = PRESET_PARAMS.find(name);
    if (it != PRESET_PARAMS.end()) {
        return it->second;
    }

    // Default to standard parameters if not found
    return PRESET_PARAMS.at("standard");
}

// Get all available evaluation function names
std::vector<std::string> getAvailableEvaluationNames() {
    std::vector<std::string> names;
    names.reserve(PRESET_PARAMS.size());

    for (const auto& [name, _] : PRESET_PARAMS) {
        names.push_back(name);
    }

    return names;
}

// Load evaluation parameters from a JSON file
EvalParams loadParamsFromJsonFile(const std::string& filename) {
    EvalParams params;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return getPresetParams("standard"); // Return default parameters if file can't be opened
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }

        // Find key-value pairs in format: "key": value or key: value
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string valueStr = line.substr(colonPos + 1);

            // Remove quotes, commas, and whitespace
            key.erase(std::remove_if(key.begin(), key.end(),
                [](char c) { return c == '"' || c == ' ' || c == '\t'; }), key.end());
            valueStr.erase(std::remove_if(valueStr.begin(), valueStr.end(),
                [](char c) { return c == '"' || c == ',' || c == ' ' || c == '\t'; }), valueStr.end());

            // Convert value to integer
            try {
                uint64_t value = std::stoull(valueStr);
                params[key] = value;
            } catch (const std::exception& e) {
                std::cerr << "Error parsing value for key '" << key << "': " << e.what() << std::endl;
            }
        }
    }

    // If no valid parameters were found, return default parameters
    if (params.empty()) {
        std::cerr << "Warning: No valid parameters found in " << filename << ". Using standard parameters." << std::endl;
        return getPresetParams("standard");
    }

    return params;
}

// Save evaluation parameters to a JSON file
bool saveParamsToJsonFile(const EvalParams& params, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return false;
    }

    file << "{\n";

    bool first = true;
    for (const auto& [name, weight] : params) {
        if (!first) {
            file << ",\n";
        }
        file << "  \"" << name << "\": " << weight;
        first = false;
    }

    file << "\n}\n";

    return true;
}

// Display details of EvalParams with formatted output
std::string getEvalParamsDetails(const EvalParams& params, bool formatted) {
    std::stringstream ss;
    uint64_t totalWeight = 0;

    if (formatted) {
        ss << "Evaluation Parameters:\n";
        ss << "------------------------------------------\n";
        ss << "| Component        | Weight  | Percentage |\n";
        ss << "------------------------------------------\n";
    }

    // Calculate total weight first
    for (const auto& [name, weight] : params) {
        totalWeight += weight;
    }

    // Sort components by weight (descending) for better readability
    std::vector<std::pair<std::string, uint64_t>> sortedParams(params.begin(), params.end());
    std::sort(sortedParams.begin(), sortedParams.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (const auto& [name, weight] : sortedParams) {
        double percentage = totalWeight > 0 ? (weight * 100.0 / totalWeight) : 0.0;

        if (formatted) {
            ss << "| " << std::left << std::setw(16) << name
               << " | " << std::right << std::setw(6) << weight
               << " | " << std::fixed << std::setprecision(1) << std::setw(9) << percentage << "% |\n";
        } else {
            ss << name << ": " << weight;
            if (totalWeight > 0) {
                ss << " (" << std::fixed << std::setprecision(1) << percentage << "%)";
            }
            ss << ", ";
        }
    }

    if (formatted) {
        ss << "------------------------------------------\n";
        ss << "| Total           | " << std::setw(6) << totalWeight << " | 100.0%     |\n";
        ss << "------------------------------------------\n";
    } else if (!params.empty()) {
        // Remove trailing comma and space
        std::string result = ss.str();
        result = result.substr(0, result.length() - 2);
        return result;
    }

    return ss.str();
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

double CompositeEvaluator::evaluate(BoardState state) const {
    // Unpack the state first
    uint8_t board[4][4];
    unpackState(state, board);

    logger.debug(Logger2048::Group::Evaluation, "Evaluating board state:");
    logger.printBoard(Logger2048::Group::Evaluation, state);
    
    // Define column widths for better formatting
    const int componentWidth = 20;  // Width for component name column
    const int rawValueWidth = 12;   // Width for raw value column
    const int weightWidth = 10;     // Width for weight column
    const int weightedValueWidth = 15; // Width for weighted value column
    
    // Create a header with the specified widths
    std::stringstream header;
    header << std::left << std::setw(componentWidth) << "Component" << "| "
        << std::right << std::setw(rawValueWidth) << "Raw Value" << " | "
        << std::setw(weightWidth) << "Weight" << " | "
        << std::setw(weightedValueWidth) << "Weighted Value";
    logger.debug(Logger2048::Group::Evaluation, header.str());
    // Create a separator line
    std::string separator(componentWidth + rawValueWidth + weightWidth + weightedValueWidth + 10, '-');
    logger.debug(Logger2048::Group::Evaluation, separator);

    // Apply each component
    double totalScore = 0;

    for (const auto& component : components) {
        double componentScore = component.function(board);
        double weightedValue = componentScore * component.weight;
        totalScore += weightedValue;

        std::stringstream line;
        line << std::left << std::setw(componentWidth) << component.name << "| "
                << std::right << std::setw(rawValueWidth) << std::fixed << std::setprecision(4) << componentScore << " | "
                << std::setw(weightWidth) << component.weight << " | "
                << std::setw(weightedValueWidth) << std::fixed << std::setprecision(4) << weightedValue;
                
        logger.debug(Logger2048::Group::Evaluation, line.str());
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

// Convert EvalParams to a simple string representation
std::string evalParamsToString(const EvalParams& params) {
    std::stringstream ss;
    
    ss << "{";
    bool first = true;
    
    // Sort components alphabetically for consistent output
    std::vector<std::pair<std::string, uint64_t>> sortedParams(params.begin(), params.end());
    std::sort(sortedParams.begin(), sortedParams.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    
    for (const auto& [name, weight] : sortedParams) {
        if (!first) {
            ss << ", ";
        }
        ss << name << ": " << weight;
        first = false;
    }
    
    ss << "}";
    return ss.str();
}

} // namespace Evaluation