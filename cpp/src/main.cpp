// main.cpp
#include "game.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

int main(int argc, char* argv[]) {
    std::string playerType = (argc > 1) ? argv[1] : "Random";
    
    if (playerType != "Random" && playerType != "Heuristic" && playerType != "MCTS") {
        std::cout << "Usage: " << argv[0] << " [player_type] [num_games]\n"
                  << "Available players:\n"
                  << "  Random     - Makes random valid moves (default)\n"
                  << "  Heuristic  - Uses heuristic evaluation\n"
                  << "  MCTS       - Uses Monte Carlo Tree Search\n";
        return 1;
    }

    int numGames = (argc > 2) ? std::stoi(argv[2]) : 1000;
    const int PROGRESS_INTERVAL = 1;  // Print progress every game

    Game2048 game;
    std::unique_ptr<Player> player;

    if (playerType == "Heuristic") {
        player = std::make_unique<HeuristicPlayer>();
    } else if (playerType == "MCTS") {
        player = std::make_unique<MCTSPlayer>();
    } else {
        player = std::make_unique<RandomPlayer>();
    }
    game.setPlayer(std::move(player));

    int bestScore = 0;
    uint64_t bestState = 0;
    int bestMoveCount = 0;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastUpdateTime = startTime;

    std::cout << "Starting " << numGames << " games with " << playerType << "...\n";

    for (int i = 0; i < numGames; ++i) {
        // Print progress
        if (i % PROGRESS_INTERVAL == 0) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto gameTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUpdateTime).count();

            if (i > 0) {
                std::cout << "\rGame " << i << "/" << numGames
                         << " (Best: " << bestScore
                         << ", Last game time: " << gameTime << "s)"
                         << std::flush;
            }
            lastUpdateTime = currentTime;
        }

        auto [score, state, moveCount] = game.playGame();

        if (score > bestScore) {
            bestScore = score;
            bestState = state;
            bestMoveCount = moveCount;
            // Print immediately when we find a new best score
            std::cout << "\nNew best score: " << bestScore
                     << " (moves: " << moveCount << ")\n" << std::flush;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "\n\nFinal Results:\n";
    std::cout << std::string(20, '-') << "\n";

    // Format duration output
    if (duration.count() > 5000) {
        double seconds = duration.count() / 1000.0;
        std::cout << "Played " << numGames << " games in "
                 << std::fixed << std::setprecision(2) << seconds << "s\n";
        std::cout << "Average time per game: "
                 << std::fixed << std::setprecision(2) << (seconds / numGames) << "s\n";
    } else {
        std::cout << "Played " << numGames << " games in " << duration.count() << "ms\n";
        std::cout << "Average time per game: "
                 << std::fixed << std::setprecision(2)
                 << (static_cast<double>(duration.count()) / numGames) << "ms\n";
    }

    std::cout << "Best score: " << bestScore << " (moves: " << bestMoveCount << ")\n";
    std::cout << "Best board:\n";

    Game2048 tmpGame;
    tmpGame.reset();
    tmpGame.setState(bestState);
    tmpGame.prettyPrint();

    return 0;
}
