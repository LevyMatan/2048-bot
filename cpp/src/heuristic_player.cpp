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

// Constructor that uses a named evaluation function
HeuristicPlayer::HeuristicPlayer(const Evaluation::EvalParams& params) {
    customName = "Heuristic";
    // Set up the evaluation function
    Evaluation::CompositeEvaluator evaluator(params);
    evalFn = [evaluator](uint64_t state) {
        return evaluator.evaluate(state);
    };
}

// Constructor with pre-configured evaluation function
HeuristicPlayer::HeuristicPlayer(const Evaluation::EvaluationFunction& fn) {
    customName = "Heuristic";
    evalFn = fn;
}

std::string HeuristicPlayer::getName() const {
    return customName;
}

std::tuple<Action, BoardState, int> HeuristicPlayer::chooseAction(BoardState state) {
    auto validActions = Board::getValidMoveActionsWithScores(state);
    if (validActions.empty()) {
        return {Action::INVALID, state, 0};
    }

    Action bestAction = Action::INVALID;
    double bestEval = 0;
    BoardState bestState = state;
    int bestMoveScore = 0;

    for (const auto& [action, nextState, moveScore] : validActions) {
        double eval = evalFn(nextState);

        if (eval > bestEval) {
            bestEval = eval;
            bestAction = action;
            bestState = nextState;
            bestMoveScore = moveScore;
        }
    }

    return {bestAction, bestState, bestMoveScore};
}
