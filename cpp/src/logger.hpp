#pragma once
#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include "board.hpp"

namespace Logger2048 {

enum class Level {
    Error,
    Warning,
    Info,
    Debug
};

enum class Group {
    Board,
    Evaluation,
    AI,
    Game,
    COUNT  // Keep this last for array size
};

struct LoggerConfig {
    Level level = Level::Info;
    std::array<bool, static_cast<size_t>(Group::COUNT)> groupsEnabled = {true, true, true, true};
    bool waitEnabled = false;
    bool shrinkBoard = false;
    bool logToFile = false;
    bool logToConsole = true;
    std::string logFile = "log.txt";
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void configure(const LoggerConfig& config);

    template<typename... Args>
    void error(Group group, Args... args);

    template<typename... Args>
    void warning(Group group, Args... args);

    template<typename... Args>
    void info(Group group, Args... args);

    template<typename... Args>
    void debug(Group group, Args... args);

    void printBoard(Group group, uint64_t board);
    void wait();

private:
    Logger() = default;
    ~Logger() {
        if (fileStream.is_open()) {
            fileStream.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template<typename... Args>
    void log(Level level, Group group, Args... args);

    bool shouldLog(Level level, Group group);
    std::string groupToString(Group group);
    std::string levelToString(Level level);

    LoggerConfig config;
    std::ofstream fileStream;
    std::mutex logMutex;
};

} // namespace Logger2048