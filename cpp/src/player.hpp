#pragma once
#include <vector>
#include <cstdint>
#include <utility>
#include <string>
#include <memory>
#include <random>
#include "board.hpp"

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
    struct Weights {
        uint64_t emptyTiles;
        uint64_t monotonicity;
        uint64_t smoothness;
        uint64_t cornerPlacement;

        Weights(uint64_t e = 250, uint64_t m = 250,
                uint64_t s = 250, uint64_t c = 250);
    };

    explicit HeuristicPlayer(const Weights& w = Weights());

    // Constructor that loads weights from a file
    explicit HeuristicPlayer(const std::string& weightsFile);

    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override;

    // Static method to load weights from a file
    static Weights loadWeightsFromFile(const std::string& filename);

private:
    Weights weights;
    std::string customName;

    uint64_t evaluatePosition(uint64_t state) const;
    uint64_t evaluateMonotonicity(const int tileValues[4][4]) const;
    uint64_t evaluateSmoothness(const int tileValues[4][4]) const;
    uint64_t evaluateCornerPlacement(const int tileValues[4][4]) const;
};

class MCTSNode {
public:
    uint64_t state;
    int visits;
    double totalScore;
    std::vector<std::unique_ptr<MCTSNode>> children;
    MCTSNode* parent;
    bool isChanceNode;

    MCTSNode(uint64_t s, MCTSNode* p = nullptr, bool chance = false);

    double UCB1(double C = 1.41) const;
};

class MCTSPlayer : public Player {
private:
    int simulations;
    std::mt19937 rng;

    MCTSNode* select(MCTSNode* node);
    void expand(MCTSNode* node);
    double evaluate(uint64_t state) const;
    double simulate(uint64_t state);
    void backpropagate(MCTSNode* node, double score);
    double getExplorationConstant(int depth) const;

public:
    explicit MCTSPlayer(int sims = 2000);
    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override;
};

// Optional: Implement DQN player later
class DQNPlayer : public Player {
public:
    explicit DQNPlayer(const std::string& model_path);
    std::tuple<Action, uint64_t, int> chooseAction(uint64_t state) override;
    std::string getName() const override { return "DQN"; }
private:
    // This is a placeholder for the DQN model
    // You'll need to integrate with your preferred ML framework
    void* model;  // Replace with actual ML model type
};