// main.cpp
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>  // Add this for std::setprecision
#include "game.hpp"
#include "player.hpp"
#include "board.hpp"

// Performance test function
void runPerformanceTest() {
    std::cout << "Running performance test..." << std::endl;
    
    // Create a board with a fixed state for consistent testing
    uint64_t state = 0x0000000100020003;  // Some fixed state
    
    const int iterations = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Perform a computationally intensive operation many times
    int totalScore = 0;
    for (int i = 0; i < iterations; i++) {
        auto moves = Board::simulateMovesWithScores(state);
        for (const auto& move : moves) {
            totalScore += std::get<1>(move);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    
    std::cout << "Performance test completed in " << elapsed.count() << "ms" << std::endl;
    std::cout << "Total score (to prevent optimization): " << totalScore << std::endl;
}

int main(int argc, char* argv[]) {
    // Check if we're running the performance test
    if (argc > 1 && std::string(argv[1]) == "perf") {
        runPerformanceTest();
        return 0;
    }
    
    std::string playerType = (argc > 1) ? argv[1] : "Random";
    std::string weightsFile = "";
    
    // Check if a weights file is provided for Heuristic player
    if (playerType == "Heuristic" && argc > 3) {
        weightsFile = argv[3];
    }
    
    if (playerType != "Random" && playerType != "Heuristic" && playerType != "MCTS") {
        std::cout << "Usage: " << argv[0] << " [player_type] [num_games] [weights_file]\n"
                  << "Available players:\n"
                  << "  Random     - Makes random valid moves (default)\n"
                  << "  Heuristic  - Uses heuristic evaluation\n"
                  << "              Optional: provide a weights file as third argument\n"
                  << "  MCTS       - Uses Monte Carlo Tree Search\n";
        return 1;
    }

    int numGames = (argc > 2) ? std::stoi(argv[2]) : 1000;
    const int PROGRESS_INTERVAL = 1;  // Print progress every game

    Game2048 game;
    std::unique_ptr<Player> player;

    // Create the appropriate player based on user input
    if (playerType == "Heuristic") {
        if (!weightsFile.empty()) {
            std::cout << "Using custom weights from file: " << weightsFile << std::endl;
            player = std::make_unique<HeuristicPlayer>(weightsFile);
        } else {
            player = std::make_unique<HeuristicPlayer>();
        }
    } else if (playerType == "MCTS") {
        player = std::make_unique<MCTSPlayer>();
    } else {
        player = std::make_unique<RandomPlayer>();
    }

    int bestScore = 0;
    uint64_t bestState = 0;
    int bestMoveCount = 0;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastUpdateTime = startTime;

    std::cout << "Starting " << numGames << " games with " << player->getName() << "...\n";

    // Create a lambda that captures the player and calls its chooseAction method
    auto chooseActionFn = [&player](uint64_t state) {
        return player->chooseAction(state);
    };

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

        auto [score, state, moveCount] = game.playGame(chooseActionFn);

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
    
    // Create a temporary game to display the best board
    Game2048 tempGame;
    tempGame.setState(bestState);
    // Set the score and move count for the best game
    tempGame.setScore(bestScore);
    tempGame.setMoveCount(bestMoveCount);
    tempGame.prettyPrint();

    return 0;
}
