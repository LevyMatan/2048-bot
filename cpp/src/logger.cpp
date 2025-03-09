#include "logger.hpp"
#include <iomanip>
#include <chrono>
#include <ctime>
#include <fstream>
#include <algorithm>

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

bool Logger::loadConfigFromJsonFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open logger configuration file: " << filename << std::endl;
        return false;
    }

    LoggerConfig newConfig;
    std::string line;
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

    // Parse level
    std::string levelStr = parseValue("level");
    if (!levelStr.empty()) {
        newConfig.level = stringToLevel(levelStr);
    }

    // Parse groupsEnabled
    for (size_t i = 0; i < static_cast<size_t>(Group::COUNT); i++) {
        std::string groupName = groupToString(static_cast<Group>(i));
        std::transform(groupName.begin(), groupName.end(), groupName.begin(), 
                      [](char c) -> char { return static_cast<char>(std::tolower(c)); });
        std::string groupEnabledStr = parseValue("enable" + groupName);
        if (!groupEnabledStr.empty()) {
            newConfig.groupsEnabled[i] = (groupEnabledStr == "true");
        }
    }

    // Parse other boolean values
    std::string waitEnabledStr = parseValue("waitEnabled");
    if (!waitEnabledStr.empty()) {
        newConfig.waitEnabled = (waitEnabledStr == "true");
    }

    std::string shrinkBoardStr = parseValue("shrinkBoard");
    if (!shrinkBoardStr.empty()) {
        newConfig.shrinkBoard = (shrinkBoardStr == "true");
    }

    std::string logToFileStr = parseValue("logToFile");
    if (!logToFileStr.empty()) {
        newConfig.logToFile = (logToFileStr == "true");
    }

    std::string logToConsoleStr = parseValue("logToConsole");
    if (!logToConsoleStr.empty()) {
        newConfig.logToConsole = (logToConsoleStr == "true");
    }
    
    // Parse showTimestamp flag
    std::string showTimestampStr = parseValue("showTimestamp");
    if (!showTimestampStr.empty()) {
        newConfig.showTimestamp = (showTimestampStr == "true");
    }

    // Parse log file path
    std::string logFile = parseValue("logFile");
    if (!logFile.empty()) {
        newConfig.logFile = logFile;
    }

    // Apply the new configuration
    configure(newConfig);
    
    return true;
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

Level Logger::stringToLevel(const std::string& levelStr) {
    std::string upperLevelStr = levelStr;
    std::transform(upperLevelStr.begin(), upperLevelStr.end(), upperLevelStr.begin(), 
                  [](char c) -> char { return static_cast<char>(std::toupper(c)); });
    
    if (upperLevelStr == "ERROR") return Level::Error;
    if (upperLevelStr == "WARN" || upperLevelStr == "WARNING") return Level::Warning;
    if (upperLevelStr == "INFO") return Level::Info;
    if (upperLevelStr == "DEBUG") return Level::Debug;
    
    return Level::Info; // Default
}