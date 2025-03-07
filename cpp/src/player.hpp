#pragma once
#include <vector>
#include <cstdint>
#include <utility>
#include <string>
#include <memory>
#include <random>
#include "board.hpp"
#include "evaluation.hpp"
#include "debug_config.hpp"

class Player {
public:

    virtual ~Player() = default; // Add a virtual destructor
    Player(const DebugConfig& debugCfg = DebugConfig()) : debugConfig(debugCfg) {}
    virtual std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) = 0;
    virtual std::string getName() const = 0;
    DebugConfig debugConfig;
};

class RandomPlayer : public Player {
public:
    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override;

private:
    std::random_device rd;
    std::mt19937 rng{rd()};
};
