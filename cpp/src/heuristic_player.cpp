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

extern Logger2048::Logger &logger;

HeuristicPlayer::HeuristicPlayer(const Evaluation::EvalParams& params)
{
    customName = "Heuristic";
    logger.debug(Logger2048::Group::AI, "Creating HeuristicPlayer with params: " + Evaluation::evalParamsToString(params));
    evaluator = std::make_unique<Evaluation::CompositeEvaluator>(params);
    evalFn = [this](BoardState state) {
        return this->evaluator->evaluate(state);
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

    // Find the best move based on the evaluation function
    double bestEval = evalFn(validMoves[0].state);
    logger.debug(Logger2048::Group::AI, "Action: ", actionToString(validMoves[0].action), "Eval: ", bestEval);
    auto bestMoveIter = validMoves.begin();
    for (auto it = validMoves.begin() + 1; it != validMoves.end(); ++it) {
        double eval = evalFn(it->state);
        logger.debug(Logger2048::Group::AI, "Action: ", actionToString(it->action), "Eval: ", eval);
        if (eval > bestEval) {
            bestEval = eval;
            bestMoveIter = it;
        }
    }

    return *bestMoveIter;
}
