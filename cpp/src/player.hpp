#pragma once
#include <vector>
#include <cstdint>
#include <utility>
#include <string>
#include <memory>
#include <random>
#include "board.hpp"
#include "evaluation.hpp"

class Player {
public:
    virtual ~Player() = default;

    // Return tuple of (action, next state, move score)
    virtual std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) = 0;
    virtual std::string getName() const = 0;
};

class RandomPlayer : public Player {
public:
    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override;

private:
    std::random_device rd;
    std::mt19937 rng{rd()};
};

class HeuristicPlayer : public Player {
public:
    // Constructor using evaluation parameters
    explicit HeuristicPlayer(const Evaluation::EvalParams& params = Evaluation::EvalParams());

    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override;

private:
    std::string customName = "Heuristic";
    Evaluation::EvaluationFunction evalFn;
};
