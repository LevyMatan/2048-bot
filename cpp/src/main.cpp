// main.cpp
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <memory>
#include <algorithm>
#include "game.hpp"
#include "players.hpp"
#include "board.hpp"
#include "evaluation.hpp"
#include "arg_parser.hpp"
#include "logger.hpp"
#include "score_types.hpp"

Logger2048::Logger &logger = Logger2048::Logger::getInstance();

std::unique_ptr<Player> createPlayer(PlayerConfigurations config);

struct GameResult {
    Score::GameScore score = 0;
    int maxTileValue = 0;  // internal representation: 12 = 4K, 13 = 8K
    int moveCount = 0;
};

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
void runGamesParallel(int startIdx, int endIdx, PlayerConfigurations playerConfig,
                     GameResult* results,
                     std::atomic<Score::GameScore>& bestScore, std::atomic<BoardState>& bestState,
                     std::atomic<int>& bestMoveCount, std::mutex& printMutex,
                     std::atomic<int>& gamesCompleted, int numGames, int progressInterval,
                     BoardState initialState) {

    Game2048 game;
    std::unique_ptr<Player> player = createPlayer(playerConfig);

    auto chooseActionFn = [&player](BoardState state) {
        return player->chooseAction(state);
    };

    for (int i = startIdx; i < endIdx; ++i) {
        auto [moveCount, state, score] = game.playGame(chooseActionFn, initialState);
        int maxTile = Board::getMaxTileValue(state);

        if (results) {
            results[i].score = score;
            results[i].maxTileValue = maxTile;
            results[i].moveCount = moveCount;
        }

        Score::GameScore currentBest = bestScore.load();
        while (score > currentBest && !bestScore.compare_exchange_weak(currentBest, score)) {
        }
        if (score > currentBest) {
            bestState.store(state);
            bestMoveCount.store(moveCount);
        }

        int completed = ++gamesCompleted;
        if (completed % progressInterval == 0 || completed == numGames) {
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

        auto logPlayer = createPlayer(playerConfig);
        logger.info(Logger2048::Group::Main, "Created player of type:", logPlayer->getName());

        // Use simulation config values
        const int numGames = simConfig.numGames;
        const int numThreads = std::max(1, simConfig.numThreads);
        const int progressInterval = std::max(1, simConfig.progressInterval);
        const BoardState initialState = simConfig.initialState;
        const std::string benchmarkOutputPath = parser.getBenchmarkOutputPath();

        std::vector<GameResult> results(benchmarkOutputPath.empty() ? 0 : static_cast<size_t>(numGames));
        GameResult* resultsPtr = results.empty() ? nullptr : results.data();

        std::atomic<Score::GameScore> bestScore(0);
        std::atomic<BoardState> bestState(0);
        std::atomic<int> bestMoveCount(0);
        std::atomic<int> gamesCompleted(0);
        std::mutex printMutex;

        auto startTime = std::chrono::high_resolution_clock::now();

        logger.info(Logger2048::Group::Main, "Starting", numGames, "games with",
                   logPlayer->getName(), "using", numThreads, "threads");

        std::vector<std::thread> threads;
        int gamesPerThread = numGames / numThreads;

        for (int i = 0; i < numThreads; ++i) {
            int startIdx = i * gamesPerThread;
            int endIdx = (i == numThreads - 1) ? numGames : (i + 1) * gamesPerThread;

            threads.emplace_back(runGamesParallel, startIdx, endIdx,
                               playerConfig, resultsPtr, std::ref(bestScore),
                               std::ref(bestState), std::ref(bestMoveCount),
                               std::ref(printMutex), std::ref(gamesCompleted),
                               numGames, progressInterval,
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

        Game2048 tempGame;
        tempGame.setState(bestState.load());
        logger.printBoard(Logger2048::Group::Main, bestState.load());
        tempGame.setScore(bestScore.load());
        tempGame.setMoveCount(bestMoveCount.load());
        tempGame.prettyPrint();

        if (!benchmarkOutputPath.empty() && !results.empty()) {
            int count4K = 0, count8K = 0;
            Score::GameScore sumScore = 0;
            for (const auto& r : results) {
                if (r.maxTileValue >= 12) count4K++;
                if (r.maxTileValue >= 13) count8K++;
                sumScore += r.score;
            }
            std::vector<Score::GameScore> scores;
            scores.reserve(results.size());
            for (const auto& r : results) scores.push_back(r.score);
            std::sort(scores.begin(), scores.end());
            size_t p95Idx = static_cast<size_t>(95 * numGames / 100);
            if (p95Idx >= scores.size()) p95Idx = scores.size() - 1;
            Score::GameScore p95Score = scores[p95Idx];
            double timePerGameMs = static_cast<double>(duration.count()) / numGames;

            std::ofstream out(benchmarkOutputPath);
            if (out.is_open()) {
                out << "{\n";
                out << "  \"numGames\": " << numGames << ",\n";
                out << "  \"hitRate4K\": " << (static_cast<double>(count4K) / numGames) << ",\n";
                out << "  \"hitRate8K\": " << (static_cast<double>(count8K) / numGames) << ",\n";
                out << "  \"avgScore\": " << (static_cast<double>(sumScore) / numGames) << ",\n";
                out << "  \"p95Score\": " << p95Score << ",\n";
                out << "  \"timePerGameMs\": " << std::fixed << std::setprecision(2) << timePerGameMs << ",\n";
                out << "  \"totalTimeMs\": " << duration.count() << "\n";
                out << "}\n";
                out.close();
                logger.info(Logger2048::Group::Main, "Benchmark stats written to", benchmarkOutputPath);
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
