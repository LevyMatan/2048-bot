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

    // Open file if needed
    if (config.outputDestination == LogOutput::File || config.outputDestination == LogOutput::Both) {
        if (fileStream.is_open()) {
            fileStream.close();
        }
        fileStream.open(config.logFile, std::ios::app);
    }
}

// Implementation for stringToLogOutput
LogOutput Logger::stringToLogOutput(const std::string& outputStr) {
    std::string upperStr = outputStr;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), 
                  [](char c) -> char { return static_cast<char>(std::toupper(c)); });
    
    if (upperStr == "NONE") return LogOutput::None;
    if (upperStr == "CONSOLE") return LogOutput::Console;
    if (upperStr == "FILE") return LogOutput::File;
    if (upperStr == "BOTH") return LogOutput::Both;
    
    return LogOutput::Console; // Default
}

// Implementation for logOutputToString
std::string Logger::logOutputToString(LogOutput output) {
    switch (output) {
        case LogOutput::None: return "None";
        case LogOutput::Console: return "Console";
        case LogOutput::File: return "File";
        case LogOutput::Both: return "Both";
        default: return "Unknown";
    }
}

LoggerConfig Logger::loadConfigFromJsonFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open logger configuration file: " << filename << std::endl;
        return config; // Return current config if file can't be opened
    }

    info(Group::Game, "Loading logger configuration from:", filename);
    
    LoggerConfig newConfig = config;
    
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
            std::string lowerGroupName = groupName;
            std::transform(lowerGroupName.begin(), lowerGroupName.end(), lowerGroupName.begin(), 
                          [](char c) -> char { return static_cast<char>(std::tolower(c)); });
            
            // Check for both old and new formats
            std::string groupEnabledStr = parseValue("enable" + lowerGroupName);
            if (groupEnabledStr.empty()) {
                // Try new format (groupNameEnable)
                groupEnabledStr = parseValue(groupName + "Enable");
            }
            
            if (!groupEnabledStr.empty()) {
                newConfig.groupsEnabled[i] = (groupEnabledStr == "true");
            }
        }
    }

    // Parse the new output destination
    std::string outputDestStr = parseValue("outputDestination");
    if (!outputDestStr.empty()) {
        newConfig.outputDestination = stringToLogOutput(outputDestStr);
    } else {
        // Backward compatibility: parse logToFile and logToConsole
        std::string logToFileStr = parseValue("logToFile");
        std::string logToConsoleStr = parseValue("logToConsole");
        
        bool logToFile = false;
        bool logToConsole = true;
        
        if (!logToFileStr.empty()) {
            logToFile = (logToFileStr == "true");
        }
        
        if (!logToConsoleStr.empty()) {
            logToConsole = (logToConsoleStr == "true");
        }
        
        // Convert to the new enum
        if (logToFile && logToConsole) {
            newConfig.outputDestination = LogOutput::Both;
        } else if (logToFile) {
            newConfig.outputDestination = LogOutput::File;
        } else if (logToConsole) {
            newConfig.outputDestination = LogOutput::Console;
        } else {
            newConfig.outputDestination = LogOutput::None;
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

    return newConfig;
}

void Logger::printBoard(Group group, BoardState board) {
    if (!shouldLog(Level::Debug, group)) return;

    std::lock_guard<std::mutex> lock(logMutex);

    uint8_t unpackedBoard[4][4];
    Board::unpackState(board, unpackedBoard);

    // Create a string stream to capture the board output
    std::stringstream ss;

    if (config.shrinkBoard) {
        // Print compact board representation with 2 digits per cell
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (j > 0) ss << " ";
                if (unpackedBoard[i][j] == 0) {
                    ss << "00";
                } else {
                    ss << std::setw(2) << std::setfill('0') << static_cast<int>(unpackedBoard[i][j]);
                }
            }
            ss << std::endl;
        }
    } else {
        // Use a temporary string stream to capture Board::printBoard output
        std::streambuf* oldCoutBuf = std::cout.rdbuf(ss.rdbuf());
        Board::printBoard(unpackedBoard);
        std::cout.rdbuf(oldCoutBuf); // Restore cout's buffer
    }

    // Now handle the output based on the configuration
    if (config.outputDestination == LogOutput::File || config.outputDestination == LogOutput::Both) {
        if (fileStream.is_open()) {
            fileStream << ss.str();
            fileStream.flush();
        }
    }

    if (config.outputDestination == LogOutput::Console || config.outputDestination == LogOutput::Both) {
        std::cout << ss.str();
    }
}

void Logger::wait() {
    if (config.waitEnabled) {
        std::cout << "Press ENTER to continue..." << std::flush;
        std::cin.get();
    }
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
        case Group::Parser: return "Parser";
        case Group::Main: return "Main";
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
    info(Group::Logger, "Logger Configuration:");
    info(Group::Logger, "- Log Level:", levelToString(config.level));
    
    info(Group::Logger, "- Enabled Groups:");
    for (size_t i = 0; i < static_cast<size_t>(Group::COUNT); i++) {
        Group group = static_cast<Group>(i);
        info(Group::Logger, "  - ", groupToString(group), ":", config.groupsEnabled[i] ? "Enabled" : "Disabled");
    }
    
    info(Group::Logger, "- Output Destination:", logOutputToString(config.outputDestination));
    info(Group::Logger, "- Show Timestamp:", config.showTimestamp ? "Yes" : "No");
    
    if (config.outputDestination == LogOutput::File || config.outputDestination == LogOutput::Both) {
        info(Group::Logger, "- Log File:", config.logFile);
    }
    
    info(Group::Logger, "- Wait Enabled:", config.waitEnabled ? "Yes" : "No");
    info(Group::Logger, "- Shrink Board:", config.shrinkBoard ? "Yes" : "No");
}