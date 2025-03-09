// game.cpp
#include "game.hpp"
#include "players.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <string>
#include <sstream>

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

/**
 * @brief Executes a move in the game.
 *
 * @param action The action to be performed.
 * @param nextState The state of the board after the action.
 * @param moveScore The score obtained from the move.
 * @return true if the move is valid and executed, false otherwise.
 */
bool Game2048::playMove(Action action, uint64_t nextState, int moveScore) {
    // Validate the action and next state
    if (Action::INVALID == action) {
        return false;
    }

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

std::tuple<int, uint64_t, int> Game2048::playGame(
    std::function<ChosenActionResult(uint64_t)> chooseActionFn) {

    reset();
    bool gameOver = false;

    while (!gameOver) {
        auto [action, nextState, moveScore] = chooseActionFn(board.getState());
        gameOver = !playMove(action, nextState, moveScore);
    }

    return {score, board.getState(), moveCount};
}

void Game2048::prettyPrint() const {
    // Calculate the width needed for the board based on the highest tile
    int maxTileValue = 0;
    uint64_t state = board.getState();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int pos = (i * 4 + j) * 4;
            int value = (state >> pos) & 0xF;
            if (value > 0) {
                int tileValue = Board::valueToTile(value);
                maxTileValue = std::max(maxTileValue, tileValue);
            }
        }
    }

    // Calculate cell width based on the number of digits in the highest tile
    std::ostringstream ss;
    ss << maxTileValue;
    int cellWidth = static_cast<int>(ss.str().length()) + 2; // +2 for padding

    // Ensure cell width is at least 6 for better appearance
    cellWidth = std::max(cellWidth, 6);

    // Calculate total board width
    int boardWidth = cellWidth * 4 + 5; // 4 cells + 5 separators

    // Print header
    std::cout << std::string(boardWidth, '-') << '\n';

    // Center the score and moves information
    std::ostringstream scoreStream;
    scoreStream << "Score: " << score << " | Moves: " << moveCount;
    std::string scoreInfo = scoreStream.str();
    int padding = static_cast<int>((boardWidth - scoreInfo.length()) / 2);
    std::cout << std::string(padding, ' ') << scoreInfo << '\n';

    std::cout << std::string(boardWidth, '-') << '\n';

    // Print board
    for (int i = 0; i < 4; ++i) {
        std::cout << '|';
        for (int j = 0; j < 4; ++j) {
            int pos = (i * 4 + j) * 4;
            int value = (state >> pos) & 0xF;

            // Center the tile value in the cell
            std::string cellContent;
            if (value == 0) {
                cellContent = "";
            } else {
                std::ostringstream tileStream;
                tileStream << Board::valueToTile(value);
                cellContent = tileStream.str();
            }

            // Calculate padding for centering
            int leftPadding = static_cast<int>((cellWidth - cellContent.length()) / 2);
            int rightPadding = cellWidth - static_cast<int>(cellContent.length()) - leftPadding;

            std::cout << std::string(leftPadding, ' ') << cellContent << std::string(rightPadding, ' ') << '|';
        }
        std::cout << '\n' << std::string(boardWidth, '-') << '\n';
    }
}
