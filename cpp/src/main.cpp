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
#include <memory>
#include "game.hpp"
#include "player.hpp"
#include "expectimax_player.hpp"
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

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [player_type] [num_games] [options]\n"
              << "Available players:\n"
              << "  Random     - Makes random valid moves\n"
              << "  Heuristic  - Uses heuristic evaluation\n"
              << "              Optional: provide a weights file as third argument\n"
              << "  Expectimax - Uses expectimax search\n"
              << "              Options:\n"
              << "              -d <depth>     Search depth (default: 3)\n"
              << "              -c <coverage>  Chance coverage (default: 2)\n"
              << "              -t <time>      Time limit in ms (default: 100)\n"
              << "              -a             Enable adaptive depth\n";
}

std::unique_ptr<Player> createExpectimaxPlayer(int argc, char* argv[], int& numGames) {
    ExpectimaxPlayer::Config config;

    // Start from 2 because argv[1] is the player type
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-d" && i + 1 < argc) {
            config.depth = std::stoi(argv[++i]);
        } else if (arg == "-c" && i + 1 < argc) {
            config.chanceCovering = std::stoi(argv[++i]);
        } else if (arg == "-t" && i + 1 < argc) {
            config.timeLimit = std::stoi(argv[++i]) / 1000.0;
        } else if (arg == "-a") {
            config.adaptiveDepth = true;
        } else if (std::isdigit(arg[0])) {
            // This is the number of games
            numGames = std::stoi(arg);
        }
    }

    std::cout << "Expectimax Configuration:\n"
              << "  Depth: " << config.depth << "\n"
              << "  Chance Coverage: " << config.chanceCovering << "\n"
              << "  Time Limit: " << (config.timeLimit * 1000) << "ms\n"
              << "  Adaptive Depth: " << (config.adaptiveDepth ? "Yes" : "No") << "\n";

    return std::make_unique<ExpectimaxPlayer>(config);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string playerType = argv[1];
    int numGames = 1;  // Default value

    std::unique_ptr<Player> player;
    try {
        if (playerType == "Random") {
            player = std::make_unique<RandomPlayer>();
            if (argc >= 3) numGames = std::stoi(argv[2]);
        } else if (playerType == "Heuristic") {
            if (argc >= 4 && argv[3][0] != '-') {
                player = std::make_unique<HeuristicPlayer>(argv[3]);
            } else {
                player = std::make_unique<HeuristicPlayer>();
            }
            if (argc >= 3) numGames = std::stoi(argv[2]);
        } else if (playerType == "Expectimax") {
            player = createExpectimaxPlayer(argc, argv, numGames);
        } else {
            printUsage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        printUsage(argv[0]);
        return 1;
    }

    // Set progress interval to 10% of total games, with a minimum of 1
    const int PROGRESS_INTERVAL = std::max(1, numGames / 10);

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
