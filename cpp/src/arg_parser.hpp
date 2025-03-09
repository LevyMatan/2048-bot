#pragma once

#include <string>
#include "players.hpp"

class SimulationConfig {
public:
    SimulationConfig(int numGames = 1, int numThreads = 1, int progressInterval = 100)
        : numGames(numGames), numThreads(numThreads), progressInterval(progressInterval) {}

    int numGames;
    int numThreads;
    int progressInterval;
};

class ArgParser {
public:
    ArgParser(int argc, char* argv[]);

    SimulationConfig getSimConfig() const;
    PlayerConfigurations getPlayerConfig() const;

private:
    void parseArguments(int argc, char* argv[]);
    void parseShortFlag(const std::string& flag, const std::string& value);
    static void printHelp();

    SimulationConfig simConfig;
    PlayerConfigurations playerConfig;
};