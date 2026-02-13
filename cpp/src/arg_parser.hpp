#pragma once

#include <string>
#include <thread>
#include "logger.hpp"
#include "players.hpp"

using namespace Logger2048;

class SimulationConfig {
public:
    SimulationConfig(int numGames = 1, int numThreads = 1, int progressInterval = 100, BoardState initialState = 0)
        : numGames(numGames), numThreads(numThreads), progressInterval(progressInterval), initialState(initialState) {}

    int numGames;
    int numThreads;
    int progressInterval;
    BoardState initialState;
};

class ArgParser {
public:
    ArgParser(int argc, char* argv[]);

    SimulationConfig getSimConfig() const;
    PlayerConfigurations getPlayerConfig() const;
    LoggerConfig getLoggerConfig() const;
    bool shouldLoadLoggerConfig() const;
    std::string getLoggerConfigPath() const;
    bool shouldLoadPlayerConfig() const;
    std::string getPlayerConfigPath() const;
    /** If non-empty, write benchmark stats (8K rate, scores, etc.) to this path as JSON. */
    std::string getBenchmarkOutputPath() const { return benchmarkOutputPath; }

private:
    void parseArguments(int argc, char* argv[]);
    void parseShortFlag(const std::string& flag, const std::string& value);
    static void printHelp();
    void loadLoggerConfigIfNeeded();
    void loadSimConfigIfNeeded();
    void loadPlayerConfigIfNeeded();

    SimulationConfig simConfig;
    PlayerConfigurations playerConfig;
    LoggerConfig loggerConfig;

    bool loadLoggerConfigFromFile = false;
    std::string loggerConfigPath = "";
    const std::string defaultLoggerConfigPath = "configurations/logger_config.json";

    bool loadSimConfigFromFile = false;
    std::string simConfigPath = "";
    const std::string defaultSimConfigPath = "configurations/sim_config.json";

    bool loadPlayerConfigFromFile = false;
    std::string playerConfigPath = "";
    const std::string defaultPlayerConfigPath = "configurations/player_config.json";

    std::string benchmarkOutputPath;
};


class TuneHeuristicParams {
public:
    int populationSize = 50;
    int generations = 20;
    int gamesPerEvaluation = 100;
    double mutationRate = 0.15;
    double elitePercentage = 0.2;
    std::string outputFile = "eval_weights.csv";
    std::string bestWeightsFile = "best_eval_weights.csv";
    std::string jsonOutputFile = "best_eval_weights.json";
    bool continueFromFile = false;
    int numThreads = std::thread::hardware_concurrency();
    int verbosity = 0;
};

class TuneHeuristicParser {
public:
    TuneHeuristicParser(int argc, char* argv[]);
    void parseArguments(int argc, char* argv[]);
    TuneHeuristicParams getParams() const;
private:
    TuneHeuristicParams params;

};
