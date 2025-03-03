// game.cpp
#include "game.hpp"
#include <iostream>
#include <iomanip>

void Game2048::addRandomTile() {
    auto emptyTiles = Board::getEmptyTiles(board.getState());
    if (emptyTiles.empty()) return;

    int idx = rng() % emptyTiles.size();
    auto [row, col] = emptyTiles[idx];
    int value = (dist(rng) < 0.9) ? 1 : 2;

    board.setState(Board::setTile(board.getState(), row, col, value));
}

bool Game2048::playMove() {
    if (!player) return false;

    // Get action and next state with score
    auto validMoves = Board::getValidMoveActionsWithScores(board.getState());
    if (validMoves.empty()) {
        return false;
    }

    // Let the player choose an action
    auto [action, nextState] = player->chooseAction(board.getState());
    if (action == -1) {
        return false;
    }

    // Find the score for this action
    int moveScore = 0;
    for (const auto& [moveAction, moveState, moveScoreValue] : validMoves) {
        if (moveAction == static_cast<Action>(action) && moveState == nextState) {
            moveScore = moveScoreValue;
            break;
        }
    }

    // Update the board state and score
    board.setState(nextState);
    score += moveScore;
    moveCount++;
    
    // Add a new random tile
    addRandomTile();
    return true;
}

void Game2048::reset() {
    board.setState(0);
    moveCount = 0;
    score = 0;
    addRandomTile();
    addRandomTile();
}

std::tuple<int, uint64_t, int> Game2048::playGame() {
    reset();
    if (!player) {
        player = std::make_unique<RandomPlayer>();
        std::cout << "No player set, using random player" << std::endl;
    }
    while (playMove()) {
        // Move count is incremented in playMove
    }
    return {score, board.getState(), moveCount};
}

void Game2048::prettyPrint() const {
    std::cout << std::string(17, '-') << '\n';
    std::cout << "Score: " << score << " | Moves: " << moveCount << '\n';
    std::cout << std::string(17, '-') << '\n';
    
    uint64_t state = board.getState();
    for (int i = 0; i < 4; ++i) {
        std::cout << '|';
        for (int j = 0; j < 4; ++j) {
            int pos = (i * 4 + j) * 4;
            int value = (state >> pos) & 0xF;
            if (value == 0) {
                std::cout << std::setw(3) << ' ' << '|';
            } else {
                std::cout << std::setw(3) << (1 << value) << '|';
            }
        }
        std::cout << '\n';
        std::cout << std::string(17, '-') << '\n';
    }
}