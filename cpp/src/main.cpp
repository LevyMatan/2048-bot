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
#include "players.hpp"
#include "board.hpp"
#include "evaluation.hpp"
#include "arg_parser.hpp"
#include "logger.hpp"

Logger2048::Logger &logger = Logger2048::Logger::getInstance();

// Performance test function
void runPerformanceTest() {
    std::cout << "Running performance test..." << std::endl;

    // Create a board with a fixed state for consistent testing
    BoardState state = 0x0000000100020003;  // Some fixed state

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
                     std::atomic<int>& bestScore, std::atomic<BoardState>& bestState,
                     std::atomic<int>& bestMoveCount, std::mutex& printMutex,
                     int numGames, int PROGRESS_INTERVAL, BoardState initialState) {

    Game2048 game;

    // Create a lambda that captures the player and calls its chooseAction method
    auto chooseActionFn = [&player](BoardState state) {
        return player->chooseAction(state);
    };

    // Use a shared atomic counter for progress reporting
    static std::atomic<int> gamesCompleted(0);

    for (int i = startIdx; i < endIdx; ++i) {
        auto [score, state, moveCount] = game.playGame(chooseActionFn, initialState);

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
                     << ")\n" << std::flush;
        }
    }
}

std::unique_ptr<Player> createPlayer(PlayerConfigurations config) {
    switch (config.playerType) {
        case PlayerType::Random:
            return std::make_unique<RandomPlayer>();
        case PlayerType::Heuristic:
            {
                auto player = std::make_unique<HeuristicPlayer>(config.evalParams);
                return player;
            }
        case PlayerType::Expectimax:
            {
                auto player = std::make_unique<ExpectimaxPlayer>(config.depth, config.chanceCovering,
                                                    config.timeLimit, config.adaptiveDepth,
                                                    config.evalParams);
                return player;
            }
        default:
            throw std::invalid_argument("Invalid player type");
    }
}

int main(int argc, char* argv[]) {
    try {
        ArgParser parser(argc, argv);
        auto loggerConfig = parser.getLoggerConfig();
        logger.configure(loggerConfig);

        auto simConfig = parser.getSimConfig();
        auto playerConfig = parser.getPlayerConfig();

        // Example of using the logger with eval params
        logger.info(Logger2048::Group::Main, "Starting application with", simConfig.numGames, "games");
        logger.info(Logger2048::Group::Main, "Using player:", PlayerConfigurations::playerTypeToString(playerConfig.playerType));
        logger.info(Logger2048::Group::Main, "Evaluation parameters:", Evaluation::evalParamsToString(playerConfig.evalParams));

        if (simConfig.initialState != 0) {
            logger.info(Logger2048::Group::Main, "Using initial state:", std::hex, simConfig.initialState, std::dec);
        }

        std::unique_ptr<Player> player = createPlayer(playerConfig);
        logger.info(Logger2048::Group::Main, "Created player of type:", player->getName());

        // Use simulation config values
        const int numGames = simConfig.numGames;
        const int numThreads = simConfig.numThreads;
        const int PROGRESS_INTERVAL = simConfig.progressInterval;
        const BoardState initialState = simConfig.initialState;

        // Use atomic variables for thread safety
        std::atomic<int> bestScore(0);
        std::atomic<BoardState> bestState(0);
        std::atomic<int> bestMoveCount(0);
        std::mutex printMutex;

        auto startTime = std::chrono::high_resolution_clock::now();

        logger.info(Logger2048::Group::Main, "Starting", numGames, "games with",
                   player->getName(), "using", numThreads, "threads");

        std::vector<std::thread> threads;
        int gamesPerThread = numGames / numThreads;

        // Create and start threads
        for (int i = 0; i < numThreads; ++i) {
            int startIdx = i * gamesPerThread;
            int endIdx = (i == numThreads - 1) ? numGames : (i + 1) * gamesPerThread;

            threads.emplace_back(runGamesParallel, startIdx, endIdx,
                               std::ref(player), std::ref(bestScore),
                               std::ref(bestState), std::ref(bestMoveCount),
                               std::ref(printMutex), numGames, PROGRESS_INTERVAL,
                               initialState);
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        logger.info(Logger2048::Group::Main, "Final Results:");
        logger.info(Logger2048::Group::Main, std::string(20, '-'));

        // Format duration output
        if (duration.count() > 5000) {
            double seconds = duration.count() / 1000.0;
            logger.info(Logger2048::Group::Main, "Played", numGames, "games in", std::fixed, std::setprecision(2), seconds, "s");
            logger.info(Logger2048::Group::Main, "Average time per game:", std::fixed, std::setprecision(2), (seconds / numGames), "s");
        } else {
            logger.info(Logger2048::Group::Main, "Played", numGames, "games in", duration.count(), "ms");
            logger.info(Logger2048::Group::Main, "Average time per game:", std::fixed, std::setprecision(2), (static_cast<double>(duration.count()) / numGames), "ms");
        }

        logger.info(Logger2048::Group::Main, "Best score:", bestScore.load(), "(moves:", bestMoveCount.load(), ")");
        logger.info(Logger2048::Group::Main, "Best board:");

        // Create a temporary game to display the best board
        Game2048 tempGame;
        tempGame.setState(bestState.load());
        logger.printBoard(Logger2048::Group::Main, bestState.load());
        // Set the score and move count for the best game
        tempGame.setScore(bestScore.load());
        tempGame.setMoveCount(bestMoveCount.load());
        tempGame.prettyPrint();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
