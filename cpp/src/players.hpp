#pragma once
#include <vector>
#include <cstdint>
#include <utility>
#include <string>
#include <memory>
#include <random>
#include <chrono>
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
        : playerType(PlayerType::Random), evalParams(Evaluation::EvalParams()), depth(3), chanceCovering(1), timeLimit(1.0), adaptiveDepth(false) {}

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
