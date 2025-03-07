#include "heuristic_player.hpp"
#include "board.hpp"
#include "evaluation.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

HeuristicPlayer::HeuristicPlayer(const Evaluation::EvalParams& params, const DebugConfig& debugCfg)
    : Player(debugCfg) // Initialize the base class with debugConfig
{
    customName = "Heuristic";
    Evaluation::CompositeEvaluator evaluator(params);
    evalFn = [evaluator](uint64_t state) {
        return evaluator.evaluate(state);
    };
}

HeuristicPlayer::HeuristicPlayer(const Evaluation::EvaluationFunction& fn) : Player(), evalFn(fn) {
    customName = "Heuristic";
}

std::string HeuristicPlayer::getName() const {
    return customName;
}

std::tuple<Action, uint64_t, int> HeuristicPlayer::chooseAction(uint64_t state) {
    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return {Action::INVALID, state, 0};
    }

    // Find the move with the highest score
    auto bestMove = std::max_element(validMoves.begin(), validMoves.end(),
        [this](const auto& a, const auto& b) {
            return evalFn(std::get<1>(a)) < evalFn(std::get<1>(b));
        });

    return {std::get<0>(*bestMove), std::get<1>(*bestMove), std::get<2>(*bestMove)};
}
