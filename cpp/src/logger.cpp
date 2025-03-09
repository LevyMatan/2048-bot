#include "logger.hpp"
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace Logger2048;

void Logger::configure(const LoggerConfig& newConfig) {
    std::lock_guard<std::mutex> lock(logMutex);
    config = newConfig;

    if (config.logToFile) {
        if (fileStream.is_open()) {
            fileStream.close();
        }
        fileStream.open(config.logFile, std::ios::app);
    }
}

template<typename... Args>
void Logger::log(Level level, Group group, Args... args) {
    if (!shouldLog(level, group)) return;

    std::lock_guard<std::mutex> lock(logMutex);

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
       << "[" << levelToString(level) << "] ";

    ((ss << args << " "), ...);
    ss << std::endl;

    if (config.logToFile && fileStream.is_open()) {
        fileStream << ss.str();
        fileStream.flush();
    }

    if(config.logToConsole || level != Level::Debug) {
        std::cout << ss.str();
    }
}

void Logger::printBoard(Group group, BoardState board) {
    if (!shouldLog(Level::Info, group)) return;

    std::lock_guard<std::mutex> lock(logMutex);

    uint8_t unpackedBoard[4][4];
    Board::unpackState(board, unpackedBoard);

    if (config.shrinkBoard) {
        // Print compact board representation
        // No borders only the tile exponent values with two digits per cell

    } else {
        // Print full board representation

        Board::printBoard(unpackedBoard);
    }


}

void Logger::wait() {
    std::cout << "Press ENTER to continue..." << std::flush;
    std::cin.get();
}

bool Logger::shouldLog(Level level, Group group) {
    return static_cast<int>(level) <= static_cast<int>(config.level) && config.groupsEnabled[static_cast<int>(group)];
}

std::string Logger::groupToString(Group group) {
    switch (group) {
        case Group::Board: return "Board";
        case Group::Evaluation: return "Eval";
        case Group::AI: return "AI";
        case Group::Game: return "Game";
        default: return "Unknown";
    }
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::Error: return "ERROR";
        case Level::Warning: return "WARN";
        case Level::Info: return "INFO";
        case Level::Debug: return "DEBUG";
        default: return "UNKNOWN";
    }
}

// Template specializations for the log functions
template<typename... Args>
void Logger::error(Group group, Args... args) {
    log(Level::Error, group, args...);
}

template<typename... Args>
void Logger::warning(Group group, Args... args) {
    log(Level::Warning, group, args...);
}

template<typename... Args>
void Logger::info(Group group, Args... args) {
    log(Level::Info, group, args...);
}

template<typename... Args>
void Logger::debug(Group group, Args... args) {
    log(Level::Debug, group, args...);
}