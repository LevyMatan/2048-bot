#include "arg_parser.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>

ArgParser::ArgParser(int argc, char* argv[]) :
    simConfig(1, 1, 100),
    playerConfig(),
    loggerConfig() {
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
                        loggerConfig.logToFile = true;
                    } else if (arg == "--log-config") {
                        Logger2048::Logger::getInstance().loadConfigFromJsonFile(value);
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
            loggerConfig.logToFile = true;
        } else if (flag == "lc") {
            Logger2048::Logger::getInstance().loadConfigFromJsonFile(value);
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

Logger2048::LoggerConfig ArgParser::getLoggerConfig() const {
    return loggerConfig;
}

void ArgParser::printHelp() {
    std::cout << "Usage: 2048 [options]\n\n"
              << "Options:\n"
              << "  -p <type>        Player type (H=Heuristic, R=Random)\n"
              << "  -n <num>         Number of games to play\n"
              << "  -t <num>         Number of threads\n"
              << "  --sim            Enable simulation mode\n"
              << "  -l <level>       Log level (e=Error, w=Warning, i=Info, d=Debug)\n"
              << "  -lf <file>       Log to file\n"
              << "  -lc <file>       Load logger config from JSON file\n"
              << "  --log-level <l>  Set log level (error, warning, info, debug)\n"
              << "  --log-file <f>   Set log file\n"
              << "  --log-config <f> Load logger config from JSON file\n"
              << "  -h, --help       Show this help\n\n"
              << "Examples:\n"
              << "  2048 -p H -n 10                 (Heuristic player, 10 games)\n"
              << "  2048 -p R -t 8                  (Random player, 8 threads)\n"
              << "  2048 -p H -lc logger_config.json (Load logger config from JSON)\n"
              << std::endl;
}