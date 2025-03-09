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

    info(Group::Game, "Loading logger configuration from:", filename);
    
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

    // Function to find a nested JSON object
    auto findNestedObject = [&jsonContent](const std::string& key) -> std::string {
        size_t pos = jsonContent.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        
        pos = jsonContent.find(':', pos);
        if (pos == std::string::npos) return "";
        
        pos = jsonContent.find('{', pos);
        if (pos == std::string::npos) return "";
        
        int braceCount = 1;
        size_t endPos = pos + 1;
        
        while (braceCount > 0 && endPos < jsonContent.length()) {
            if (jsonContent[endPos] == '{') braceCount++;
            else if (jsonContent[endPos] == '}') braceCount--;
            endPos++;
        }
        
        if (braceCount != 0) return "";
        return jsonContent.substr(pos, endPos - pos);
    };

    // Parse level
    std::string levelStr = parseValue("level");
    if (!levelStr.empty()) {
        newConfig.level = stringToLevel(levelStr);
    }

    // Parse groupsEnabled from the new nested structure
    std::string groupsObject = findNestedObject("groups");
    if (!groupsObject.empty()) {
        for (size_t i = 0; i < static_cast<size_t>(Group::COUNT); i++) {
            std::string groupName = groupToString(static_cast<Group>(i));
            std::string groupEnabledStr = parseValue(groupName);
            
            // Extract directly from the groups object
            size_t pos = groupsObject.find("\"" + groupName + "\"");
            if (pos != std::string::npos) {
                pos = groupsObject.find(':', pos);
                if (pos != std::string::npos) {
                    pos = groupsObject.find_first_not_of(" \t\n\r", pos + 1);
                    if (pos != std::string::npos) {
                        std::string valueStr;
                        size_t endPos = groupsObject.find_first_of(",}", pos);
                        if (endPos != std::string::npos) {
                            valueStr = groupsObject.substr(pos, endPos - pos);
                            // Trim whitespace
                            valueStr.erase(std::remove_if(valueStr.begin(), valueStr.end(), 
                                         [](char c) { return std::isspace(c); }), valueStr.end());
                            newConfig.groupsEnabled[i] = (valueStr == "true");
                        }
                    }
                }
            }
        }
    } else {
        // Fall back to the old format for backward compatibility
        for (size_t i = 0; i < static_cast<size_t>(Group::COUNT); i++) {
            std::string groupName = groupToString(static_cast<Group>(i));
            std::transform(groupName.begin(), groupName.end(), groupName.begin(), 
                          [](char c) -> char { return static_cast<char>(std::tolower(c)); });
            std::string groupEnabledStr = parseValue("enable" + groupName);
            if (!groupEnabledStr.empty()) {
                newConfig.groupsEnabled[i] = (groupEnabledStr == "true");
            }
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
    
    // Print the configuration after configuring
    printConfiguration();
    
    return true;
}

void Logger::printBoard(Group group, BoardState board) {
    if (!shouldLog(Level::Info, group)) return;

    std::lock_guard<std::mutex> lock(logMutex);

    uint8_t unpackedBoard[4][4];
    Board::unpackState(board, unpackedBoard);

    if (config.shrinkBoard) {
        // Print compact board representation with 2 digits per cell
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (j > 0) std::cout << " ";
                if (unpackedBoard[i][j] == 0) {
                    std::cout << "00";
                } else {
                    std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(unpackedBoard[i][j]);
                }
            }
            std::cout << std::endl;
        }
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
        case Group::Logger: return "Logger";
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

void Logger::printConfiguration() {
    info(Group::Game, "Logger Configuration:");
    info(Group::Game, "- Log Level:", levelToString(config.level));
    
    info(Group::Game, "- Enabled Groups:");
    for (size_t i = 0; i < static_cast<size_t>(Group::COUNT); i++) {
        info(Group::Game, "  - ", groupToString(static_cast<Group>(i)), ":", config.groupsEnabled[i] ? "Enabled" : "Disabled");
    }
    
    info(Group::Game, "- Show Timestamp:", config.showTimestamp ? "Yes" : "No");
    info(Group::Game, "- Log to Console:", config.logToConsole ? "Yes" : "No");
    info(Group::Game, "- Log to File:", config.logToFile ? "Yes" : "No");
    
    if (config.logToFile) {
        info(Group::Game, "  - Log File:", config.logFile);
    }
    
    info(Group::Game, "- Wait Enabled:", config.waitEnabled ? "Yes" : "No");
    info(Group::Game, "- Shrink Board:", config.shrinkBoard ? "Yes" : "No");
}