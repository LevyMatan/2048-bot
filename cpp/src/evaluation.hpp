#pragma once

#include <functional>
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include "board.hpp"
#include "logger.hpp"

namespace Evaluation {

using Weight = double;

// Primary evaluation function type
using EvaluationFunction = std::function<double(BoardState)>;

// Simple evaluation function that works on unpacked board
using SimpleEvalFunc = std::function<double(const uint8_t[4][4])>;

// Parameters for weighted evaluations
using EvalParams = std::unordered_map<std::string, Weight>;

EvalParams getPresetParams(const std::string& name);
// Helper function to unpack state into a 2D board
void unpackState(BoardState state, uint8_t board[4][4]);

uint8_t findMaxTile(const uint8_t board[4][4]);

// Core simple evaluation functions
double monotonicity(const uint8_t board[4][4]);
double emptyTiles(const uint8_t board[4][4]);
double mergeability(const uint8_t board[4][4]);
double smoothness(const uint8_t board[4][4]);
double cornerValue(const uint8_t board[4][4]);
double patternMatching(const uint8_t board[4][4]);


// Get a named evaluation function
SimpleEvalFunc getNamedEvaluation(const std::string& name);

// Get preset parameter configurations
EvalParams getPresetParams(const std::string& name);

// Load evaluation parameters from a JSON file
EvalParams loadParamsFromJsonFile(const std::string& filename);

// Save evaluation parameters to a JSON file
bool saveParamsToJsonFile(const EvalParams& params, const std::string& filename);

// Get all available evaluation function names
std::vector<std::string> getAvailableEvaluationNames();

// Display details of EvalParams with formatted output
std::string getEvalParamsDetails(const EvalParams& params, bool formatted = true);
struct EvaluationComponent {
    SimpleEvalFunc function;
    Weight weight;
    std::string name;

    EvaluationComponent(SimpleEvalFunc f, Weight w, std::string n)
        : function(f), weight(w), name(n) {}
};

// Composite evaluator class
class CompositeEvaluator {
public:
    CompositeEvaluator(EvalParams params);

    // Add a component with weight
    void addComponent(SimpleEvalFunc func, Weight weight, const std::string& name);

    // Remove a component by name
    void removeComponent(const std::string& name);

    // Evaluate a state using all components
    double evaluate(BoardState state) const;

    // Get/set component weights
    void setWeight(const std::string& name, Weight weight);
    Weight getWeight(const std::string& name) const;

    // Get parameters as a map
    EvalParams getParams() const;

    // Set weights from a parameter map
    void setParams(const EvalParams& params);

    // Add this new method for detailed evaluation output
    void printDetailedEvaluation(BoardState state) const;

private:
    std::vector<EvaluationComponent> components;
    std::unordered_map<std::string, size_t> componentIndices;
};

// Convert EvalParams to a simple string representation
std::string evalParamsToString(const EvalParams& params);

} // namespace Evaluation