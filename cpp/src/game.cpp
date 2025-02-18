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

    uint64_t nextState;
    auto [action, state] = player->chooseAction(board.getState());
    if (-1 == action) {
        return false;
    }
    nextState = state;

    board.setState(nextState);
    addRandomTile();
    return true;
}

int Game2048::getScore() const {
    int score = 0;
    uint64_t state = board.getState();
    for (int i = 0; i < 16; ++i) {
        int value = (state >> (i * 4)) & 0xF;
        if (value > 0) {
            score += 1 << value;
        }
    }
    return score;
}

void Game2048::reset() {
    board.setState(0);
    moveCount = 0;
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
        moveCount++;
    }
    return {getScore(), board.getState(), moveCount};
}

void Game2048::prettyPrint() const {
    std::cout << std::string(11, '-') << '\n';
    uint64_t state = board.getState();
    for (int i = 0; i < 4; ++i) {
        uint16_t row = (state >> (48 - i * 16)) & 0xFFFF;
        std::cout << std::hex << std::uppercase
                  << ((row >> 12) & 0xF) << ' '
                  << ((row >> 8) & 0xF) << ' '
                  << ((row >> 4) & 0xF) << ' '
                  << (row & 0xF) << '\n';
    }
    std::cout << std::dec;
}