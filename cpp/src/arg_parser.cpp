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
                // Special handling for -lc, --log-config, -sc, --sim-config, -pc, --player-config which can be used without a value
                if (arg == "-lc" || arg == "--log-config") {
                    loadLoggerConfigFromFile = true;
                    
                    // Check if the next argument exists and is not a flag (doesn't start with -)
                    if (i + 1 < argc && argv[i + 1][0] != '-') {
                        loggerConfigPath = argv[++i];
                    }
                    // If not, the default path will be used
                    continue;
                }
                
                // Handle simulation config
                if (arg == "-sc" || arg == "--sim-config") {
                    loadSimConfigFromFile = true;
                    
                    // Check if the next argument exists and is not a flag (doesn't start with -)
                    if (i + 1 < argc && argv[i + 1][0] != '-') {
                        simConfigPath = argv[++i];
                    }
                    // If not, the default path will be used
                    continue;
                }
                
                // Handle player config
                if (arg == "-pc" || arg == "--player-config") {
                    loadPlayerConfigFromFile = true;
                    
                    // Check if the next argument exists and is not a flag (doesn't start with -)
                    if (i + 1 < argc && argv[i + 1][0] != '-') {
                        playerConfigPath = argv[++i];
                    }
                    // If not, the default path will be used
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
    
    // Load configurations
    loadLoggerConfigIfNeeded();
    loadSimConfigIfNeeded();
    loadPlayerConfigIfNeeded();
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
    std::cout << "Usage: 2048 [options]\n\n"
              << "Options:\n"
              << "  -p <type>        Player type (H=Heuristic, R=Random)\n"
              << "  -n <num>         Number of games to play\n"
              << "  -t <num>         Number of threads\n"
              << "  -is <hex>        Initial board state (hex uint64)\n"
              << "  --initial-state <hex> Initial board state (hex uint64)\n"
              << "  -sc [file]       Load simulation config from JSON file (default: configurations/sim_config.json)\n"
              << "  --sim-config [f] Load simulation config from JSON file (default: configurations/sim_config.json)\n"
              << "  -pc [file]       Load player config from JSON file (default: configurations/player_config.json)\n"
              << "  --player-config [f] Load player config from JSON file (default: configurations/player_config.json)\n"
              << "  --sim            Enable simulation mode\n"
              << "  -l <level>       Log level (e=Error, w=Warning, i=Info, d=Debug)\n"
              << "  -lf <file>       Log to file (output destination will include File)\n"
              << "  -lc [file]       Load logger config from JSON file (default: configurations/logger_config.json)\n"
              << "  --log-level <l>  Set log level (error, warning, info, debug)\n"
              << "  --log-file <f>   Set log file (output destination will include File)\n"
              << "  --output <dest>  Set output destination (none, console, file, both)\n"
              << "  --log-config [f] Load logger config from JSON file (default: configurations/logger_config.json)\n"
              << "  -h, --help       Show this help\n\n"
              << "Examples:\n"
              << "  2048 -p H -n 10                   (Heuristic player, 10 games)\n"
              << "  2048 -p R -t 8                    (Random player, 8 threads)\n"
              << "  2048 -p H -is 0x123456789abcdef0  (Start from specific board state)\n"
              << "  2048 -p H -sc                     (Load default simulation config)\n"
              << "  2048 -p H -lc                     (Load default logger config)\n"
              << "  2048 -p H -pc                     (Load default player config)\n"
              << std::endl;
}