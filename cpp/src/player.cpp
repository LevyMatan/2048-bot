PlayerConfigurations PlayerConfigurations::fromString(const std::string& playerString) {
    PlayerConfigurations config;
    if (playerString == "random") {
        config.playerType = PlayerType::Random;
    } else if (playerString == "heuristic") {
        config.playerType = PlayerType::Heuristic;
    } else if (playerString == "expectimax") {
        config.playerType = PlayerType::Expectimax;
    } else {
        throw std::runtime_error("Invalid player type: " + playerString);
    }
    return config;
}
PlayerConfigurations PlayerConfigurations::loadFromJsonFile(const std::string& filename) {
    PlayerConfigurations config;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    size_t pos;

    // Find the player type
    pos = jsonContent.find("\"playerType\"");
    if (pos != std::string::npos) {
        pos = jsonContent.find(":", pos);
        if (pos != std::string::npos) {
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string playerTypeStr = jsonContent.substr(pos, endPos - pos);
            playerTypeStr.erase(std::remove_if(playerTypeStr.begin(), playerTypeStr.end(),
                                                 [](char c) { return std::isspace(c) || c == '"'; }),
                                 playerTypeStr.end());
            try {
                config.playerType = playerTypeFromString(playerTypeStr);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing playerType: " << e.what() << std::endl;
            }
        }
    }

    // Find the depth
    pos = jsonContent.find("\"depth\"");
    if (pos != std::string::npos) {
        pos = jsonContent.find(":", pos);
        if (pos != std::string::npos) {
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string depthStr = jsonContent.substr(pos, endPos - pos);
            depthStr.erase(std::remove_if(depthStr.begin(), depthStr.end(),
                                             [](char c) { return std::isspace(c); }),
                             depthStr.end());
            try {
                config.depth = std::stoi(depthStr);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing depth: " << e.what() << std::endl;
            }
        }
    }

    // Find the chanceCovering
    pos = jsonContent.find("\"chanceCovering\"");
    if (pos != std::string::npos) {
        pos = jsonContent.find(":", pos);
        if (pos != std::string::npos) {
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string chanceCoveringStr = jsonContent.substr(pos, endPos - pos);
            chanceCoveringStr.erase(std::remove_if(chanceCoveringStr.begin(), chanceCoveringStr.end(),
                                             [](char c) { return std::isspace(c); }),
                             chanceCoveringStr.end());
            try {
                config.chanceCovering = std::stod(chanceCoveringStr);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing chanceCovering: " << e.what() << std::endl;
            }
        }
    }

    // Find the timeLimit
    pos = jsonContent.find("\"timeLimit\"");
    if (pos != std::string::npos) {
        pos = jsonContent.find(":", pos);
        if (pos != std::string::npos) {
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string timeLimitStr = jsonContent.substr(pos, endPos - pos);
            timeLimitStr.erase(std::remove_if(timeLimitStr.begin(), timeLimitStr.end(),
                                             [](char c) { return std::isspace(c); }),
                             timeLimitStr.end());
            try {
                config.timeLimit = std::stoi(timeLimitStr);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing timeLimit: " << e.what() << std::endl;
            }
        }
    }

    // Find the adaptiveDepth
    pos = jsonContent.find("\"adaptiveDepth\"");
    if (pos != std::string::npos) {
        pos = jsonContent.find(":", pos);
        if (pos != std::string::npos) {
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string adaptiveDepthStr = jsonContent.substr(pos, endPos - pos);
            adaptiveDepthStr.erase(std::remove_if(adaptiveDepthStr.begin(), adaptiveDepthStr.end(),
                                             [](char c) { return std::isspace(c); }),
                             adaptiveDepthStr.end());
            try {
                std::istringstream(adaptiveDepthStr) >> std::boolalpha >> config.adaptiveDepth;
            } catch (const std::exception& e) {
                std::cerr << "Error parsing adaptiveDepth: " << e.what() << std::endl;
            }
        }
    }

    // Find the evaluation parameters
    pos = jsonContent.find("\"evalParams\"");
    if (pos != std::string::npos) {
        pos = jsonContent.find("{", pos);
        if (pos != std::string::npos) {
            size_t startPos = pos;
            int braceCount = 1;
            pos++;

            // Find the matching closing brace
            while (braceCount > 0 && pos < jsonContent.size()) {
                if (jsonContent[pos] == '{') braceCount++;
                else if (jsonContent[pos] == '}') braceCount--;
                pos++;
            }

            if (braceCount == 0) {
                // Extract the evalParams JSON object
                std::string evalParamsJson = jsonContent.substr(startPos, pos - startPos);

                // Parse each parameter
                size_t paramPos = 0;
                while ((paramPos = evalParamsJson.find("\"", paramPos)) != std::string::npos) {
                    paramPos++; // Skip opening quote
                    size_t nameEndPos = evalParamsJson.find("\"", paramPos);
                    if (nameEndPos == std::string::npos) break;
                    std::string paramName = evalParamsJson.substr(paramPos, nameEndPos - paramPos);

                    paramPos = evalParamsJson.find(":", nameEndPos);
                    if (paramPos == std::string::npos) break;

                    paramPos = evalParamsJson.find_first_not_of(" \t\n\r", paramPos + 1);
                    if (paramPos == std::string::npos) break;
                    size_t valueEndPos = evalParamsJson.find_first_of(",}", paramPos);
                    if (valueEndPos == std::string::npos) break;
                    std::string valueStr = evalParamsJson.substr(paramPos, valueEndPos - paramPos);

                    // Remove any whitespace from the value
                    valueStr.erase(std::remove_if(valueStr.begin(), valueStr.end(),
                                                     [](char c) { return std::isspace(c); }),
                                     valueStr.end());

                    try {
                        config.evalParams[paramName] = std::stod(valueStr);
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing evalParam '" << paramName << "': " << e.what() << std::endl;
                    }

                    paramPos = valueEndPos;
                }
            }
        }
    }

    return config;
}
