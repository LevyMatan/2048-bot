#pragma once

#include <string>
#include "players.hpp"
#include "logger.hpp"

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

private:
    void parseArguments(int argc, char* argv[]);
    void parseShortFlag(const std::string& flag, const std::string& value);
    static void printHelp();
    void loadLoggerConfigIfNeeded();
    void loadSimConfigIfNeeded();

    SimulationConfig simConfig;
    PlayerConfigurations playerConfig;
    LoggerConfig loggerConfig;
    
    bool loadLoggerConfigFromFile = false;
    std::string loggerConfigPath = "";
    const std::string defaultLoggerConfigPath = "configurations/logger_config.json";
    
    bool loadSimConfigFromFile = false;
    std::string simConfigPath = "";
    const std::string defaultSimConfigPath = "configurations/sim_config.json";
};