#pragma once
#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include <iomanip>
#include <chrono>
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
    Logger,
    Parser,
    Main,
    Tuner,
    COUNT  // Keep this last for array size
};

enum class LogOutput {
    None,      // No output
    Console,   // Console only
    File,      // File only
    Both       // Both console and file
};

struct LoggerConfig {
    Level level = Level::Error;
    std::array<bool, static_cast<size_t>(Group::COUNT)> groupsEnabled = {
        false,  // Board
        false,  // Evaluation
        false,  // AI
        false,  // Game
        false,  // Logger
        false,  // Parser
        false   // Main
    };
    bool waitEnabled = false;
    bool shrinkBoard = false;
    LogOutput outputDestination = LogOutput::None;
    bool showTimestamp = false;
    std::string logFile = "log.txt";
};

#ifdef NDEBUG   // Release build: disable logging
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    template<typename... Args>
    inline void error(Group, Args&&...) {}
    template<typename... Args>
    inline void warning(Group, Args&&...) {}
    template<typename... Args>
    inline void info(Group, Args&&...) {}
    template<typename... Args>
    inline void debug(Group, Args&&...) {}
    inline void printBoard(Group, uint64_t) {}
    inline void wait() {}
    inline LoggerConfig loadConfigFromJsonFile(const std::string&) { return LoggerConfig{}; }
    inline void configure(const LoggerConfig&) {}
    inline void printConfiguration() {}
    static inline LogOutput stringToLogOutput(const std::string&) { return LogOutput::None; }
private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
#else   // Debug build: include full logging implementation
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void configure(const LoggerConfig& config);

    LoggerConfig loadConfigFromJsonFile(const std::string& filename);

    void printConfiguration();

    template<typename... Args>
    void error(Group group, Args... args) {
        log(Level::Error, group, args...);
    }

    template<typename... Args>
    void warning(Group group, Args... args) {
        log(Level::Warning, group, args...);
    }

    template<typename... Args>
    void info(Group group, Args... args) {
        log(Level::Info, group, args...);
    }

    template<typename... Args>
    void debug(Group group, Args... args) {
        log(Level::Debug, group, args...);
    }

    void printBoard(Group group, BoardState board);
    void wait();

    const LoggerConfig& getConfig() const { return config; }

    static LogOutput stringToLogOutput(const std::string& outputStr);

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
    void log(Level level, Group group, Args... args) {
        if (!shouldLog(level, group)) return;

        std::lock_guard<std::mutex> lock(logMutex);
        std::stringstream ss;
        if (config.showTimestamp) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm time_info;
#ifdef _WIN32
            localtime_s(&time_info, &time);
#else
            localtime_r(&time, &time_info);
#endif
            ss << "[" << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << "] ";
        }
        ss << "[" << levelToString(level) << "] ";
        ((ss << args << " "), ...);
        ss << std::endl;

        if ((config.outputDestination == LogOutput::File || config.outputDestination == LogOutput::Both) && fileStream.is_open()) {
            fileStream << ss.str();
            fileStream.flush();
        }
        if (config.outputDestination == LogOutput::Console ||
            config.outputDestination == LogOutput::Both ||
            level != Level::Debug) {
            std::cout << ss.str();
        }
    }

    bool shouldLog(Level level, Group group);
    std::string groupToString(Group group);
    std::string levelToString(Level level);
    static Level stringToLevel(const std::string& levelStr);
    std::string logOutputToString(LogOutput output);

    LoggerConfig config;
    std::ofstream fileStream;
    std::mutex logMutex;
};
#endif  // NDEBUG

} // namespace Logger2048