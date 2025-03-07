#pragma once

#include "player.hpp"
#include "evaluation.hpp"
#include <chrono>
#include <functional>
#include <string>
#include <random>
#include <unordered_map>

typedef uint64_t BoardState;

typedef struct {
    uint8_t depth;
    double heuristic;
} trans_table_entry_t;

typedef std::unordered_map<BoardState, trans_table_entry_t> trans_table_t;

class ExpectimaxPlayer : public Player {
public:
    // Type alias for evaluation function
    using EvaluationFunction = Evaluation::EvaluationFunction;

    struct Config {
        int depth;              // Search depth
        int chanceCovering;     // Number of random tile possibilities to consider
        double timeLimit;       // Time limit per move in seconds
        bool adaptiveDepth;     // Whether to use dynamic depth adjustment
        std::string evalName;   // Name of the evaluation function to use
        std::string jsonFile;   // Path to a JSON file containing custom evaluation parameters

        Config(int d = 4, int c = 3, double t = 0.2, bool a = true, std::string e = "combined", std::string j = "")
            : depth(d), chanceCovering(c), timeLimit(t), adaptiveDepth(a), evalName(e), jsonFile(j) {}
    };

    // Constructors
    ExpectimaxPlayer(const Config& config = Config());
    ExpectimaxPlayer(const Config& config, EvaluationFunction evalFn);

    // Implement Player interface
    std::tuple<Action, BoardState, int> chooseAction(BoardState state) override;
    std::string getName() const override {
        return "Expectimax-" + config.evalName;
    }

private:
    Config config;
    EvaluationFunction evalFn;
    trans_table_t trans_table;
    int cacheHits;
    int depthLimit;
    std::chrono::steady_clock::time_point startTime;
    std::random_device rd;
    std::mt19937 rng;

    double expectimax(BoardState state, int depth, bool isMax);
    double chanceNode(BoardState state, int depth, double prob);
    double maxNode(BoardState state, int depth, double prob);

    bool shouldTimeOut() const;
    int getAdaptiveDepth(BoardState state) const;
};