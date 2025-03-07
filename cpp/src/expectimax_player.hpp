#pragma once

#include "player.hpp"
#include "evaluation.hpp"
#include "debug_config.hpp"
#include "board.hpp"
#include <chrono>
#include <functional>
#include <string>
#include <random>
#include <unordered_map>
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
        int depth;
        int chanceCovering;
        double timeLimit;
        bool adaptiveDepth;
        std::string evalName;
        std::string jsonFile;

        Config() :
            depth(3),
            chanceCovering(3),
            timeLimit(0.2),
            adaptiveDepth(false),
            evalName("combined"),
            jsonFile("")
        {}
    };

    // Constructors
    ExpectimaxPlayer(const Config& config = Config(), const DebugConfig& debugCfg = DebugConfig());
    ExpectimaxPlayer(const Config& config, EvaluationFunction evalFn, const DebugConfig& debugCfg);

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