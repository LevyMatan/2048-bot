#pragma once
#include "player.hpp"
#include "evaluation.hpp"
#include "debug_config.hpp"

class HeuristicPlayer : public Player {
public:
    // Constructor using evaluation parameters
    explicit HeuristicPlayer(const Evaluation::EvalParams& params = Evaluation::EvalParams(), const DebugConfig& debugCfg = DebugConfig());

    // Constructor with pre-configured evaluation function
    explicit HeuristicPlayer(const Evaluation::EvaluationFunction& fn);

    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override;

private:
    std::string customName = "Heuristic";
    Evaluation::EvaluationFunction evalFn;
};