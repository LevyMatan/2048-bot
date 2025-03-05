#pragma once
#include "board.hpp"
#include "player.hpp"
#include <functional>

class ExpectimaxPlayer : public Player {
public:
    // Type alias for evaluation function
    using EvaluationFunction = std::function<uint64_t(uint64_t)>;

    struct Config {
        int depth;              // Search depth
        int chanceCovering;     // Number of random tile possibilities to consider
        double timeLimit;       // Time limit per move in seconds
        bool adaptiveDepth;     // Whether to use dynamic depth adjustment

        Config(int d = 3, int c = 2, double t = 0.1, bool a = true)
            : depth(d), chanceCovering(c), timeLimit(t), adaptiveDepth(a) {}
    };

    ExpectimaxPlayer(const Config& config = Config(),
                     EvaluationFunction evalFn = nullptr);

    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override { return "Expectimax"; }

    // Predefined evaluation functions
    static uint64_t defaultEvaluation(uint64_t state);
    static uint64_t monotonicEvaluation(uint64_t state);
    static uint64_t cornerEvaluation(uint64_t state);

private:
    Config config;
    EvaluationFunction evalFn;
    std::chrono::steady_clock::time_point startTime;

    double expectimax(uint64_t state, int depth, bool isMax);
    double chanceNode(uint64_t state, int depth);
    double maxNode(uint64_t state, int depth);
    bool shouldTimeOut() const;
    int getAdaptiveDepth(uint64_t state) const;
};