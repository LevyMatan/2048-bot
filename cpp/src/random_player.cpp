#include "player.hpp"
#include "board.hpp"

std::pair<int, uint64_t> RandomPlayer::chooseAction(uint64_t state) {
    auto validActions = Board::getValidMoveActions(state);
    if (validActions.empty()) {
        return {-1, state};
    }
    
    std::uniform_int_distribution<> dist(0, static_cast<int>(validActions.size()) - 1);
    int idx = dist(rng);
    auto [action, nextState] = validActions[idx];
    
    return {static_cast<int>(action), nextState};
}

std::string RandomPlayer::getName() const {
    return "Random";
} 