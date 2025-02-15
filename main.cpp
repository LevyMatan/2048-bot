// main.cpp
#include "game.hpp"
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
    int numGames = (argc > 1) ? std::stoi(argv[1]) : 1000;

    Game2048 game;
    int bestScore = 0;
    uint64_t bestState = 0;
    int bestMoveCount = 0;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numGames; ++i) {
        auto [score, state, moveCount] = game.playGame();
        if (score > bestScore) {
            bestScore = score;
            bestState = state;
            bestMoveCount = moveCount;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Played " << numGames << " games in " << duration.count() << "ms\n";
    std::cout << "Best score: " << bestScore << " (moves: " << bestMoveCount << ")\n";
    std::cout << "Best board:\n";

    Game2048 tmpGame;
    tmpGame.reset();
    tmpGame.setState(bestState);  // Instead of tmpGame.board.setState(bestState)
    tmpGame.prettyPrint();

    return 0;
}
