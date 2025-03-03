// game.cpp
#include "game.hpp"
#include <iostream>
#include <iomanip>

void Game2048::addRandomTile() {
    auto emptyTiles = Board::getEmptyTiles(board.getState());
    if (emptyTiles.empty()) return;

    // Choose a random empty tile
    int idx = static_cast<int>(dist(rng) * emptyTiles.size());
    auto [row, col] = emptyTiles[idx];

    // 90% chance of a 2, 10% chance of a 4
    int value = (dist(rng) < 0.9) ? 1 : 2;
    board.setState(Board::setTile(board.getState(), row, col, value));
}

bool Game2048::playMove(int action, uint64_t nextState) {
    // Get action and next state with score
    auto validMoves = Board::getValidMoveActionsWithScores(board.getState());
    if (validMoves.empty()) {
        return false;
    }

    // Validate the action and next state
    if (action == -1) {
        return false;
    }

    // Find the score for this action
    int moveScore = 0;
    for (const auto& [moveAction, moveState, moveScoreValue] : validMoves) {
        if (moveAction == static_cast<Action>(action) && moveState == nextState) {
            moveScore = moveScoreValue;
            board.setState(nextState);
            score += moveScore;
            moveCount++;
            
            // Add a new random tile
            addRandomTile();
            return true;
        }
    }

    // If we get here, the action was invalid
    return false;
}

void Game2048::reset() {
    board.setState(0);
    moveCount = 0;
    score = 0;
    addRandomTile();
    addRandomTile();
}

std::tuple<int, uint64_t, int> Game2048::playGame(std::function<std::pair<int, uint64_t>(uint64_t)> chooseActionFn) {
    reset();
    
    while (true) {
        auto validMoves = Board::getValidMoveActionsWithScores(board.getState());
        if (validMoves.empty()) {
            break;
        }
        
        // Let the provided function choose an action
        auto [action, nextState] = chooseActionFn(board.getState());
        
        // Apply the move
        if (!playMove(action, nextState)) {
            break;
        }
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
                std::cout << std::setw(4) << " " << '|';
            } else {
                std::cout << std::setw(4) << Board::valueToTile(value) << '|';
            }
        }
        std::cout << '\n' << std::string(17, '-') << '\n';
    }
}