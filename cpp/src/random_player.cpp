#include "player.hpp"
#include "board.hpp"

std::pair<int, uint64_t> RandomPlayer::chooseAction(uint64_t state) {
    auto validActions = Board::getValidMoveActions(state);
    if (validActions.empty()) {
        return {-1, state};
    }
    int randomIdx = rng() % validActions.size();
    return validActions[randomIdx];
} 