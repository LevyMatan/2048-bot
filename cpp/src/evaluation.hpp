#pragma once

#include <functional>
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>

namespace Evaluation {

// Primary evaluation function type
using EvaluationFunction = std::function<uint64_t(uint64_t)>;

// Simple evaluation function that works on unpacked board
using SimpleEvalFunc = std::function<uint64_t(const uint8_t[4][4])>;

// Parameters for weighted evaluations
using EvalParams = std::unordered_map<std::string, uint64_t>;

EvalParams getPresetParams(const std::string& name);
// Helper function to unpack state into a 2D board
void unpackState(uint64_t state, uint8_t board[4][4]);

uint8_t findMaxTile(const uint8_t board[4][4]);

// Core simple evaluation functions
uint64_t monotonicity(const uint8_t board[4][4]);
uint64_t emptyTiles(const uint8_t board[4][4]);
uint64_t mergeability(const uint8_t board[4][4]);
uint64_t smoothness(const uint8_t board[4][4]);
uint64_t cornerValue(const uint8_t board[4][4]);
uint64_t patternMatching(const uint8_t board[4][4]);


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

// Component for building composite evaluations
using Weight = uint64_t;
using BoardValue = uint64_t;

struct EvaluationComponent {
    SimpleEvalFunc function;
    Weight weight;
    std::string name;

    EvaluationComponent(SimpleEvalFunc f, uint64_t w, std::string n)
        : function(f), weight(w), name(n) {}
};

// Composite evaluator class
class CompositeEvaluator {
public:
    CompositeEvaluator(EvalParams params);

    // Add a component with weight
    void addComponent(SimpleEvalFunc func, uint64_t weight, const std::string& name);

    // Evaluate a state using all components
    uint64_t evaluate(uint64_t state) const;

    // Get/set component weights
    void setWeight(const std::string& name, uint64_t weight);
    uint64_t getWeight(const std::string& name) const;

    // Get parameters as a map
    EvalParams getParams() const;

    // Set weights from a parameter map
    void setParams(const EvalParams& params);

private:
    std::vector<EvaluationComponent> components;
    std::unordered_map<std::string, size_t> componentIndices;
};

static constexpr size_t BOARD_SIZE = 4;
static constexpr uint64_t MIN_WEIGHT = 0;
static constexpr uint64_t MAX_WEIGHT = UINT64_MAX;

// Add debugging utilities:
struct EvaluationBreakdown {
    std::string componentName;
    uint64_t rawScore;
    uint64_t weightedScore;
};

std::vector<EvaluationBreakdown> getDetailedEvaluation(uint64_t state);

} // namespace Evaluation