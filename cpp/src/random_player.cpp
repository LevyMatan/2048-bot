#include "player.hpp"
#include "board.hpp"

std::tuple<Action, uint64_t, int> RandomPlayer::chooseAction(uint64_t state) {
    auto validActions = Board::getValidMoveActionsWithScores(state);
    if (validActions.empty()) {
        return {Action::INVALID, state, 0};
    }

    std::uniform_int_distribution<> dist(0, static_cast<int>(validActions.size()) - 1);
    int idx = dist(rng);
    return validActions[idx];
}

std::string RandomPlayer::getName() const {
    return "Random";
}