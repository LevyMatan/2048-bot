#include "arg_parser.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>

ArgParser::ArgParser(int argc, char* argv[]) :
    simConfig(1, 1, 100),
    playerConfig() {
    parseArguments(argc, argv);
}

void ArgParser::parseArguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printHelp();
            exit(0);
        }

        try {
            if (i + 1 >= argc && arg[0] == '-') {
                throw std::runtime_error("Missing value for flag: " + arg);
            }

            if (arg[0] == '-') {
                std::string value = argv[++i];
                if (arg[1] == '-') { // Long format
                    if (arg == "--player") {
                        playerConfig.playerType = PlayerConfigurations::playerTypeFromString(value);
                    } else if (arg == "--sim") {
                        simConfig.progressInterval = 100;
                    }
                } else { // Short format
                    parseShortFlag(arg.substr(1), value);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            printHelp();
            exit(1);
        }
    }
}

void ArgParser::parseShortFlag(const std::string& flag, const std::string& value) {
    try {
        if (flag == "n") {
            simConfig.numGames = std::stoi(value);
        } else if (flag == "t") {
            simConfig.numThreads = std::stoi(value);
        } else if (flag == "p") {
            playerConfig = PlayerConfigurations::fromString(value);
        } else {
            throw std::runtime_error("Unknown flag: -" + flag);
        }
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid value for -" + flag + ": " + value);
    }
}

SimulationConfig ArgParser::getSimConfig() const {
    return simConfig;
}

PlayerConfigurations ArgParser::getPlayerConfig() const {
    return playerConfig;
}

void ArgParser::printHelp() {
    std::cout << "Usage: 2048 [options]\n\n"
              << "Options:\n"
              << "  -p <type>       Player type (H=Heuristic, R=Random)\n"
              << "  -n <num>        Number of games to play\n"
              << "  -t <num>        Number of threads\n"
              << "  --sim           Enable simulation mode\n"
              << "  -h, --help      Show this help\n\n"
              << "Examples:\n"
              << "  2048 -p H -n 10     (Heuristic player, 10 games)\n"
              << "  2048 -p R -t 8      (Random player, 8 threads)\n"
              << std::endl;
}