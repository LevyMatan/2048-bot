#pragma once
#include <vector>
#include <cstdint>
#include <utility>
#include <string>
#include <memory>
#include <random>
#include <chrono>
#include <fstream>
#include "board.hpp"
#include "evaluation.hpp"

/**
 * @brief
 *
 */
enum class PlayerType {
    Random,
    Heuristic,
    Expectimax
};

/**
 * @brief Player configurations
 * - Which player to use
 * - Evaluation params
 * - Specific player configurations
 *
 */
class PlayerConfigurations {
public:
    PlayerType playerType;
    Evaluation::EvalParams evalParams;
    int depth;
    int chanceCovering;
    double timeLimit;
    bool adaptiveDepth;

    PlayerConfigurations(PlayerType type, Evaluation::EvalParams params, int d, int chance, double time, bool adaptive)
        : playerType(type), evalParams(params), depth(d), chanceCovering(chance), timeLimit(time), adaptiveDepth(adaptive) {}

    // Default constructor with default values
    PlayerConfigurations()
        : playerType(PlayerType::Heuristic), evalParams(Evaluation::EvalParams()), depth(3), chanceCovering(1), timeLimit(1.0), adaptiveDepth(false) {}

    static PlayerType playerTypeFromString(const std::string& str) {
        if (str == "R") {
            return PlayerType::Random;
        } else if (str == "H") {
            return PlayerType::Heuristic;
        } else if (str == "E") {
            return PlayerType::Expectimax;
        } else {
            throw std::invalid_argument("Invalid player type");
        }
    }

    static std::string playerTypeToString(PlayerType type) {
        switch (type) {
            case PlayerType::Random:
                return "Random";
            case PlayerType::Heuristic:
                return "Heuristic";
            case PlayerType::Expectimax:
                return "Expectimax";
            default:
                return "Unknown";
        }
    }

    static PlayerConfigurations fromString(const std::string& type) {
        PlayerConfigurations config;
        config.playerType = playerTypeFromString(type);

        if (type == "H") {
            config.depth = 6;
            config.adaptiveDepth = true;
        } else if (type == "E") {
            config.depth = 6;
            config.chanceCovering = 4;
            config.timeLimit = 100.0;
            config.adaptiveDepth = true;
        }

        return config;
    }

    // Load player configuration from a JSON file
    static PlayerConfigurations loadFromJsonFile(const std::string& filename) {
        PlayerConfigurations config;

        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open player config file: " + filename);
        }

        std::string line, jsonContent;
        while (std::getline(file, line)) {
            jsonContent += line;
        }

        size_t pos = 0;

        // Find the playerType
        pos = jsonContent.find("\"playerType\"");
        if (pos != std::string::npos) {
            pos = jsonContent.find(":", pos);
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            if (jsonContent[pos] == '"') {
                // String value
                pos++; // Skip opening quote
                size_t endPos = jsonContent.find("\"", pos);
                std::string typeStr = jsonContent.substr(pos, endPos - pos);

                // Map "Random" to "R", etc.
                if (typeStr == "Random") {
                    config.playerType = PlayerType::Random;
                } else if (typeStr == "Heuristic") {
                    config.playerType = PlayerType::Heuristic;
                } else if (typeStr == "Expectimax") {
                    config.playerType = PlayerType::Expectimax;
                } else {
                    throw std::runtime_error("Invalid player type: " + typeStr);
                }
            }
        }

        // Find the depth
        pos = jsonContent.find("\"depth\"");
        if (pos != std::string::npos) {
            pos = jsonContent.find(":", pos);
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string valueStr = jsonContent.substr(pos, endPos - pos);
            config.depth = std::stoi(valueStr);
        }

        // Find the chanceCovering
        pos = jsonContent.find("\"chanceCovering\"");
        if (pos != std::string::npos) {
            pos = jsonContent.find(":", pos);
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string valueStr = jsonContent.substr(pos, endPos - pos);
            config.chanceCovering = std::stoi(valueStr);
        }

        // Find the timeLimit
        pos = jsonContent.find("\"timeLimit\"");
        if (pos != std::string::npos) {
            pos = jsonContent.find(":", pos);
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string valueStr = jsonContent.substr(pos, endPos - pos);
            config.timeLimit = std::stod(valueStr);
        }

        // Find the adaptiveDepth
        pos = jsonContent.find("\"adaptiveDepth\"");
        if (pos != std::string::npos) {
            pos = jsonContent.find(":", pos);
            pos = jsonContent.find_first_not_of(" \t\n\r", pos + 1);
            size_t endPos = jsonContent.find_first_of(",}", pos);
            std::string valueStr = jsonContent.substr(pos, endPos - pos);
            config.adaptiveDepth = (valueStr == "true");
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
                        std::string paramName = evalParamsJson.substr(paramPos, nameEndPos - paramPos);

                        paramPos = evalParamsJson.find(":", nameEndPos);
                        if (paramPos == std::string::npos) break;

                        paramPos = evalParamsJson.find_first_not_of(" \t\n\r", paramPos + 1);
                        size_t valueEndPos = evalParamsJson.find_first_of(",}", paramPos);
                        std::string valueStr = evalParamsJson.substr(paramPos, valueEndPos - paramPos);
                        // Remove any whitespace from the value
                        valueStr.erase(std::remove_if(valueStr.begin(), valueStr.end(),
                        [](char c) { return std::isspace(c); }), valueStr.end());

                        // Store the parameter
                        config.evalParams[paramName] = std::stod(valueStr);
                        paramPos = valueEndPos;
                    }
                }
            }
        }

        return config;
    }
};

class Player {
public:

    virtual ~Player() = default;
    Player(){}
    virtual ChosenActionResult chooseAction(BoardState state) = 0;
    virtual std::string getName() const = 0;
    std::function<ChosenActionResult(BoardState)> getDecisionFn() {
        return [this](BoardState state) { return this->chooseAction(state); };
    }
};

class RandomPlayer : public Player {
public:
    ChosenActionResult chooseAction(BoardState state) override;
    std::string getName() const override;

private:
    std::random_device rd;
    std::mt19937 rng{rd()};
};

class HeuristicPlayer : public Player {
    public:
        // Constructor using evaluation parameters
        explicit HeuristicPlayer(const Evaluation::EvalParams& params = Evaluation::EvalParams());

        // Constructor with pre-configured evaluation function
        explicit HeuristicPlayer(const Evaluation::EvaluationFunction& fn);

        ChosenActionResult chooseAction(BoardState state) override;
        std::string getName() const override;

    private:
        std::string customName = "Heuristic";
        Evaluation::EvaluationFunction evalFn;
        std::unique_ptr<Evaluation::CompositeEvaluator> evaluator;
    };


class ExpectimaxPlayer : public Player {
    public:
        // Type alias for evaluation function
        using EvaluationFunction = Evaluation::EvaluationFunction;

        // Constructors
        ExpectimaxPlayer(const int depth, const int chanceCovering, const double timeLimit, const bool adaptive_depth, const Evaluation::EvalParams& params = Evaluation::EvalParams());

        // Implement Player interface
        ChosenActionResult chooseAction(BoardState state) override;
        std::string getName() const override {
            return "Expectimax";
        }

    private:
        int depthLimit;
        int chanceCovering;
        double timeLimit;
        bool adaptiveDepth;
        std::chrono::time_point<std::chrono::steady_clock> startTime;
        Evaluation::EvalParams evalParams;
        Evaluation::CompositeEvaluator evaluator;
        std::function<double(BoardState)> evalFn;
        std::random_device rd;
        std::mt19937 rng;

        double expectimax(BoardState state, int depth, bool isMax);
        double chanceNode(BoardState state, int depth, double prob);
        double maxNode(BoardState state, int depth, double prob);

        bool shouldTimeOut() const;
        int getAdaptiveDepth(BoardState state) const;
};
