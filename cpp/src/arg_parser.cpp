#include "arg_parser.hpp"
#include "logger.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>

extern Logger2048::Logger &logger;

ArgParser::ArgParser(int argc, char* argv[]) :
    simConfig(1, 1, 100, 0),
    playerConfig(),
    loggerConfig() {
    // Set default player type to Heuristic if not specified
    playerConfig.playerType = PlayerType::Heuristic;

    // Parse command line arguments
    parseArguments(argc, argv);

    // Load configurations from files if needed
    loadLoggerConfigIfNeeded();
    loadSimConfigIfNeeded();
    loadPlayerConfigIfNeeded();
}

void ArgParser::parseArguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printHelp();
            exit(0);
        }

        try {
            if (arg[0] == '-') {
                // Special handling for flags that can be used without a value

                // Logger config flags
                if (arg == "-lc" || arg == "--log-config" || arg == "--logger-config") {
                    loadLoggerConfigFromFile = true;

                    // Check if the next argument exists and is not a flag (doesn't start with -)
                    if (i + 1 < argc && argv[i + 1][0] != '-') {
                        loggerConfigPath = argv[++i];
                    }
                    // If not, the default path will be used
                    continue;
                }

                // Simulation config flags
                if (arg == "-sc" || arg == "--sim-config" || arg == "--sim") {
                    loadSimConfigFromFile = true;

                    // Check if the next argument exists and is not a flag (doesn't start with -)
                    if (i + 1 < argc && argv[i + 1][0] != '-') {
                        simConfigPath = argv[++i];
                    }
                    // If not, the default path will be used
                    continue;
                }

                // Player config flags
                if (arg == "-pc" || arg == "--player-config") {
                    loadPlayerConfigFromFile = true;

                    // Check if the next argument exists and is not a flag (doesn't start with -)
                    if (i + 1 < argc && argv[i + 1][0] != '-') {
                        playerConfigPath = argv[++i];
                    }
                    // If not, the default path will be used
                    continue;
                }

                // Boolean flags that don't need values
                if (arg == "--wait") {
                    loggerConfig.waitEnabled = true;
                    continue;
                }
                if (arg == "--timestamp") {
                    loggerConfig.showTimestamp = true;
                    continue;
                }
                if (arg == "--compact") {
                    loggerConfig.shrinkBoard = true;
                    continue;
                }
                if (arg == "--adaptive") {
                    playerConfig.adaptiveDepth = true;
                    continue;
                }

                // For all other flags, require a value
                if (i + 1 >= argc) {
                    throw std::runtime_error("Missing value for flag: " + arg);
                }

                std::string value = argv[++i];
                if (arg[1] == '-') { // Long format
                    if (arg == "--player") {
                        playerConfig.playerType = PlayerConfigurations::playerTypeFromString(value);
                    } else if (arg == "--sim") {
                        simConfig.progressInterval = 100;
                    } else if (arg == "--log-level") {
                        if (value == "error") {
                            loggerConfig.level = Logger2048::Level::Error;
                        } else if (value == "warning") {
                            loggerConfig.level = Logger2048::Level::Warning;
                        } else if (value == "info") {
                            loggerConfig.level = Logger2048::Level::Info;
                        } else if (value == "debug") {
                            loggerConfig.level = Logger2048::Level::Debug;
                        } else {
                            throw std::runtime_error("Invalid log level: " + value);
                        }
                    } else if (arg == "--log-file") {
                        loggerConfig.logFile = value;
                        loggerConfig.outputDestination = (loggerConfig.outputDestination == LogOutput::Console ||
                                                         loggerConfig.outputDestination == LogOutput::None) ?
                                                         LogOutput::File : LogOutput::Both;
                    } else if (arg == "--initial-state") {
                        // Parse hexadecimal state
                        try {
                            simConfig.initialState = std::stoull(value, nullptr, 16);
                        } catch (const std::exception& _e) {
                            throw std::runtime_error("Invalid initial state: " + value + " " + _e.what());
                        }
                    } else if (arg == "--output") {
                        loggerConfig.outputDestination = Logger2048::Logger::stringToLogOutput(value);
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
        } else if (flag == "l") {
            if (value == "e") {
                loggerConfig.level = Logger2048::Level::Error;
            } else if (value == "w") {
                loggerConfig.level = Logger2048::Level::Warning;
            } else if (value == "i") {
                loggerConfig.level = Logger2048::Level::Info;
            } else if (value == "d") {
                loggerConfig.level = Logger2048::Level::Debug;
            } else {
                throw std::runtime_error("Invalid log level: " + value);
            }
        } else if (flag == "lf") {
            loggerConfig.logFile = value;
            loggerConfig.outputDestination = (loggerConfig.outputDestination == LogOutput::Console ||
                                             loggerConfig.outputDestination == LogOutput::None) ?
                                             LogOutput::File : LogOutput::Both;
        } else if (flag == "is") {
            // Parse hexadecimal state
            try {
                simConfig.initialState = std::stoull(value, nullptr, 16);
            } catch (const std::exception& _e) {
                throw std::runtime_error("Invalid initial state: " + value + " " + _e.what());
            }
        } else if (flag == "o") {
            loggerConfig.outputDestination = Logger2048::Logger::stringToLogOutput(value);
        } else {
            throw std::runtime_error("Unknown flag: -" + flag);
        }
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid value for -" + flag + ": " + value);
    }
}

void ArgParser::loadLoggerConfigIfNeeded() {
    if (loadLoggerConfigFromFile) {
        std::string configPath = loggerConfigPath.empty() ? defaultLoggerConfigPath : loggerConfigPath;
        loggerConfig = logger.loadConfigFromJsonFile(configPath);
    }
}

void ArgParser::loadSimConfigIfNeeded() {
    if (loadSimConfigFromFile) {
        std::string configPath = simConfigPath.empty() ? defaultSimConfigPath : simConfigPath;

        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open simulation configuration file: " << configPath << std::endl;
            return;
        }

        logger.info(Logger2048::Group::Parser, "Loading simulation configuration from:", configPath);

        std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // Simple JSON parsing
        auto parseValue = [&jsonContent](const std::string& key) -> std::string {
            size_t pos = jsonContent.find("\"" + key + "\"");
            if (pos == std::string::npos) return "";

            pos = jsonContent.find(':', pos);
            if (pos == std::string::npos) return "";

            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            if (pos == std::string::npos) return "";

            // Check if value is wrapped in quotes
            if (jsonContent[pos] == '"') {
                size_t endPos = jsonContent.find('"', pos + 1);
                if (endPos == std::string::npos) return "";
                return jsonContent.substr(pos + 1, endPos - pos - 1);
            }
            else {
                // Number or boolean value
                size_t endPos = jsonContent.find_first_of(",}\n", pos);
                if (endPos == std::string::npos) endPos = jsonContent.length();
                std::string value = jsonContent.substr(pos, endPos - pos);
                // Trim whitespace
                value.erase(std::remove_if(value.begin(), value.end(),
                             [](char c) { return std::isspace(c); }), value.end());
                return value;
            }
        };

        // Parse numGames
        std::string numGamesStr = parseValue("numGames");
        if (!numGamesStr.empty()) {
            try {
                simConfig.numGames = std::stoi(numGamesStr);
            } catch (const std::exception&) {
                logger.warning(Logger2048::Group::Parser, "Invalid numGames in config:", numGamesStr);
            }
        }

        // Parse numThreads
        std::string numThreadsStr = parseValue("numThreads");
        if (!numThreadsStr.empty()) {
            try {
                simConfig.numThreads = std::stoi(numThreadsStr);
            } catch (const std::exception&) {
                logger.warning(Logger2048::Group::Parser, "Invalid numThreads in config:", numThreadsStr);
            }
        }

        // Parse progressInterval
        std::string progressIntervalStr = parseValue("progressInterval");
        if (!progressIntervalStr.empty()) {
            try {
                simConfig.progressInterval = std::stoi(progressIntervalStr);
            } catch (const std::exception&) {
                logger.warning(Logger2048::Group::Parser, "Invalid progressInterval in config:", progressIntervalStr);
            }
        }

        // Parse initialState (as hex string)
        std::string initialStateStr = parseValue("initialState");
        if (!initialStateStr.empty()) {
            try {
                simConfig.initialState = std::stoull(initialStateStr, nullptr, 16);
            } catch (const std::exception&) {
                logger.warning(Logger2048::Group::Parser, "Invalid initialState in config:", initialStateStr);
            }
        }

        logger.info(Logger2048::Group::Logger, "Simulation Configuration:");
        logger.info(Logger2048::Group::Logger, "- Num Games:", simConfig.numGames);
        logger.info(Logger2048::Group::Logger, "- Num Threads:", simConfig.numThreads);
        logger.info(Logger2048::Group::Logger, "- Progress Interval:", simConfig.progressInterval);
        logger.info(Logger2048::Group::Logger, "- Initial State:", std::hex, simConfig.initialState, std::dec);
    }
}

void ArgParser::loadPlayerConfigIfNeeded() {
    if (loadPlayerConfigFromFile) {
        std::string configToLoad = playerConfigPath.empty() ? defaultPlayerConfigPath : playerConfigPath;
        logger.info(Logger2048::Group::Parser, "Loading player configuration from:", configToLoad);

        try {
            playerConfig = PlayerConfigurations::loadFromJsonFile(configToLoad);
            logger.info(Logger2048::Group::Parser, "Successfully loaded player configuration");
            logger.debug(Logger2048::Group::Parser, "Player type:", PlayerConfigurations::playerTypeToString(playerConfig.playerType));
            logger.debug(Logger2048::Group::Parser, "Eval params:", Evaluation::evalParamsToString(playerConfig.evalParams));
            logger.debug(Logger2048::Group::Parser, "Depth:", playerConfig.depth);
            logger.debug(Logger2048::Group::Parser, "Chance covering:", playerConfig.chanceCovering);
            logger.debug(Logger2048::Group::Parser, "Time limit:", playerConfig.timeLimit);
            logger.debug(Logger2048::Group::Parser, "Adaptive depth:", playerConfig.adaptiveDepth ? "Yes" : "No");
        } catch (const std::exception& e) {
            logger.error(Logger2048::Group::Parser, "Failed to load player configuration:", e.what());
            logger.warning(Logger2048::Group::Parser, "Using default player configuration");
        }
    }
}

SimulationConfig ArgParser::getSimConfig() const {
    return simConfig;
}

PlayerConfigurations ArgParser::getPlayerConfig() const {
    return playerConfig;
}

LoggerConfig ArgParser::getLoggerConfig() const {
    return loggerConfig;
}

bool ArgParser::shouldLoadLoggerConfig() const {
    return loadLoggerConfigFromFile;
}

std::string ArgParser::getLoggerConfigPath() const {
    return loggerConfigPath.empty() ? defaultLoggerConfigPath : loggerConfigPath;
}

bool ArgParser::shouldLoadPlayerConfig() const {
    return loadPlayerConfigFromFile;
}

std::string ArgParser::getPlayerConfigPath() const {
    return playerConfigPath.empty() ? defaultPlayerConfigPath : playerConfigPath;
}

void ArgParser::printHelp() {
    std::cout << "2048 Game Bot - A C++ implementation of 2048 with AI players\n\n"
              << "Usage: 2048 [options]\n\n"
              << "GAME OPTIONS:\n"
              << "  -n, --games <num>      Number of games to play (default: 1)\n"
              << "  -t, --threads <num>    Number of parallel threads (default: 1)\n"
              << "  -i, --initial <hex>    Initial board state as hex (default: random)\n"
              << "  --progress <num>       Progress reporting interval (default: 100)\n"
              << "  --sim <file>           Load simulation settings from JSON file\n"
              << "\n"
              << "PLAYER OPTIONS:\n"
              << "  -p, --player <type>    Player type: random, heuristic, expectimax (default: heuristic)\n"
              << "  -d, --depth <num>      Search depth for AI (default: depends on player)\n"
              << "  -c, --chance <num>     Chance node coverage (for expectimax)\n"
              << "  --time <ms>            Time limit per move in milliseconds\n"
              << "  --adaptive             Enable adaptive search depth\n"
              << "  --player-config <file> Load player settings from JSON file\n"
              << "\n"
              << "LOGGING OPTIONS:\n"
              << "  -l, --log-level <lvl>  Log level: error, warn, info, debug (default: info)\n"
              << "  -o, --output <dest>    Output destination: none, console, file, both (default: console)\n"
              << "  -f, --file <path>      Log file path (default: game.log)\n"
              << "  --wait                 Wait for keypress between moves (debug mode)\n"
              << "  --timestamp            Show timestamps in logs\n"
              << "  --compact              Use compact board representation in logs\n"
              << "  --logger-config <file> Load logger settings from JSON file\n"
              << "\n"
              << "GENERAL OPTIONS:\n"
              << "  -h, --help             Show this help message\n"
              << "  -v, --version          Show version information\n"
              << "\n"
              << "EXAMPLES:\n"
              << "  Play 10 games with the heuristic player:\n"
              << "    2048 --games 10 --player heuristic\n"
              << "\n"
              << "  Play with expectimax player, depth 4, and log to file:\n"
              << "    2048 --player expectimax --depth 4 --output file --file my_games.log\n"
              << "\n"
              << "  Load configurations from JSON files:\n"
              << "    2048 --player-config configs/player.json --logger-config configs/logger.json\n"
              << "\n"
              << "  Debug mode with detailed output and waiting between moves:\n"
              << "    2048 --log-level debug --wait --output both\n";
}


TuneHeuristicParser::TuneHeuristicParser(int argc, char* argv[]) {
    // Parse command line arguments
    parseArguments(argc, argv);

    // Initialize logger
    logger.info(Logger2048::Group::Parser, "Tuning Parameters:");
    logger.info(Logger2048::Group::Parser, "  Population Size: " + std::to_string(params.populationSize));
    logger.info(Logger2048::Group::Parser, "  Generations: " + std::to_string(params.generations));
    logger.info(Logger2048::Group::Parser, "  Games per Evaluation: " + std::to_string(params.gamesPerEvaluation));
    logger.info(Logger2048::Group::Parser, "  Mutation Rate: " + std::to_string(params.mutationRate));
    logger.info(Logger2048::Group::Parser, "  Elite Percentage: " + std::to_string(params.elitePercentage));
    logger.info(Logger2048::Group::Parser, "  Output File: " + params.outputFile);
}

void TuneHeuristicParser::parseArguments(int argc, char* argv[]) {

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p" && i + 1 < argc) {
            params.populationSize = std::stoi(argv[++i]);
        } else if (arg == "-g" && i + 1 < argc) {
            params.generations = std::stoi(argv[++i]);
        } else if (arg == "-n" && i + 1 < argc) {
            params.gamesPerEvaluation = std::stoi(argv[++i]);
        } else if (arg == "-m" && i + 1 < argc) {
            params.mutationRate = std::stod(argv[++i]);
        } else if (arg == "-e" && i + 1 < argc) {
            params.elitePercentage = std::stod(argv[++i]);
        } else if (arg == "-o" && i + 1 < argc) {
            params.outputFile = argv[++i];
        } else if (arg == "-b" && i + 1 < argc) {
            params.bestWeightsFile = argv[++i];
        } else if (arg == "-j" && i + 1 < argc) {
            params.jsonOutputFile = argv[++i];
        } else if (arg == "-c") {
            params.continueFromFile = true;
        } else if (arg == "-t" && i + 1 < argc) {
            params.numThreads = std::stoi(argv[++i]);
        } else if (arg == "-v" && i + 1 < argc) {
            params.verbosity = std::stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: tune_heuristic [options]\n"
                      << "Options:\n"
                      << "  -p <size>       Population size (default: 50)\n"
                      << "  -g <num>        Number of generations (default: 20)\n"
                      << "  -n <num>        Games per evaluation (default: 100)\n"
                      << "  -m <rate>       Mutation rate (default: 0.15)\n"
                      << "  -e <percent>    Elite percentage (default: 0.2)\n"
                      << "  -o <file>       Output file (default: eval_weights.csv)\n"
                      << "  -b <file>       Best weights file (default: best_eval_weights.csv)\n"
                      << "  -j <file>       JSON output file (default: best_eval_weights.json)\n"
                      << "  -c              Continue from file\n"
                      << "  -t <threads>    Number of threads (default: CPU cores)\n"
                      << "  -v <level>      Verbosity level (0-2, default: 0)\n"
                      << "  -h, --help      Show this help message\n";
            exit(0);
        }
    }
}

TuneHeuristicParams TuneHeuristicParser::getParams() const {
    return params;
}