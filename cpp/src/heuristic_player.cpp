#include "players.hpp"
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

HeuristicPlayer::HeuristicPlayer(const Evaluation::EvalParams& params)
{
    customName = "Heuristic";
    Evaluation::CompositeEvaluator evaluator(params);
    evalFn = [evaluator](BoardState state) {
        return evaluator.evaluate(state);
    };
}

HeuristicPlayer::HeuristicPlayer(const Evaluation::EvaluationFunction& fn) : evalFn(fn) {
    customName = "Heuristic";
}

std::string HeuristicPlayer::getName() const {
    return customName;
}

ChosenActionResult HeuristicPlayer::chooseAction(BoardState state) {
    auto validMoves = Board::getValidMoveActionsWithScores(state);
    if (validMoves.empty()) {
        return {Action::INVALID, state, 0};
    }

    // Find the move with the highest score
    auto bestMoveIter = std::max_element(validMoves.begin(), validMoves.end(),
        [this](const auto& a, const auto& b) {
            return evalFn(a.state) < evalFn(b.state);
        });

    return *bestMoveIter;
}
