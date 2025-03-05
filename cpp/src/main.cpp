// main.cpp
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>  // Add this for std::setprecision
#include <thread>   // Add this for std::thread
#include <mutex>    // Add this for std::mutex
#include <vector>   // Already included, but making it explicit
#include <atomic>   // Add this for std::atomic
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

// Function to run games in parallel
void runGamesParallel(int startIdx, int endIdx, std::unique_ptr<Player>& player,
                     std::atomic<int>& bestScore, std::atomic<uint64_t>& bestState,
                     std::atomic<int>& bestMoveCount, std::mutex& printMutex,
                     int numGames, int PROGRESS_INTERVAL) {

    Game2048 game;

    // Create a lambda that captures the player and calls its chooseAction method
    auto chooseActionFn = [&player](uint64_t state) {
        return player->chooseAction(state);
    };

    // Use a shared atomic counter for progress reporting
    static std::atomic<int> gamesCompleted(0);

    for (int i = startIdx; i < endIdx; ++i) {
        auto [score, state, moveCount] = game.playGame(chooseActionFn);

        // Update best score if better (using atomic compare-exchange)
        int currentBest = bestScore.load();
        while (score > currentBest && !bestScore.compare_exchange_weak(currentBest, score)) {
            // Keep trying if another thread updated the value
        }

        if (score > currentBest) {
            bestState.store(state);
            bestMoveCount.store(moveCount);
        }

        // Increment shared counter and print progress
        int completed = ++gamesCompleted;
        if (completed % PROGRESS_INTERVAL == 0 || completed == numGames) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "\rGame " << completed << "/" << numGames
                     << " (Best: " << bestScore.load()
                     << ")" << std::flush;
        }
    }
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
                  << "              Optional: provide a weights file as third argument\n";
        return 1;
    }

    int numGames = (argc > 2) ? std::stoi(argv[2]) : 1000;

    // Set progress interval to 10% of total games, with a minimum of 1
    const int PROGRESS_INTERVAL = std::max(1, numGames / 10);

    std::unique_ptr<Player> player;

    // Create the appropriate player based on user input
    if (playerType == "Heuristic") {
        if (!weightsFile.empty()) {
            std::cout << "Using custom weights from file: " << weightsFile << std::endl;
            player = std::make_unique<HeuristicPlayer>(weightsFile);
        } else {
            player = std::make_unique<HeuristicPlayer>();
        }
    } else {
        player = std::make_unique<RandomPlayer>();
    }

    // Use atomic variables for thread safety
    std::atomic<int> bestScore(0);
    std::atomic<uint64_t> bestState(0);
    std::atomic<int> bestMoveCount(0);
    std::mutex printMutex;

    auto startTime = std::chrono::high_resolution_clock::now();

    std::cout << "Starting " << numGames << " games with " << player->getName() << "...\n";

    // Determine number of threads based on hardware
    unsigned int numThreads = std::thread::hardware_concurrency();
    // Ensure at least 2 threads and not more than 8 (to avoid excessive thread creation)
    numThreads = std::max(2u, std::min(8u, numThreads));

    // For small number of games, use fewer threads
    if (numGames < 100) {
        numThreads = std::min(numThreads, static_cast<unsigned int>(numGames));
    }

    std::vector<std::thread> threads;
    int gamesPerThread = numGames / numThreads;

    // Create and start threads
    for (unsigned int i = 0; i < numThreads; ++i) {
        int startIdx = i * gamesPerThread;
        int endIdx = (i == numThreads - 1) ? numGames : (i + 1) * gamesPerThread;

        threads.emplace_back(runGamesParallel, startIdx, endIdx, std::ref(player),
                            std::ref(bestScore), std::ref(bestState),
                            std::ref(bestMoveCount), std::ref(printMutex),
                            numGames, PROGRESS_INTERVAL);
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
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

    std::cout << "Best score: " << bestScore.load() << " (moves: " << bestMoveCount.load() << ")\n";
    std::cout << "Best board:\n";

    // Create a temporary game to display the best board
    Game2048 tempGame;
    tempGame.setState(bestState.load());
    // Set the score and move count for the best game
    tempGame.setScore(bestScore.load());
    tempGame.setMoveCount(bestMoveCount.load());
    tempGame.prettyPrint();

    return 0;
}
