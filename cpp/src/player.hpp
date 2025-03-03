#pragma once
#include <vector>
#include <cstdint>
#include <utility>
#include <string>
#include <memory>
#include <random>

class Player {
public:
    virtual ~Player() = default;
    virtual std::pair<int, uint64_t> chooseAction(uint64_t state) = 0;
    virtual std::string getName() const = 0;
    
    // Default implementation for getMove that uses chooseAction
    virtual int getMove(uint64_t state) {
        return chooseAction(state).first;
    }
};

class RandomPlayer : public Player {
public:
    std::pair<int, uint64_t> chooseAction(uint64_t state) override;
    std::string getName() const override;

private:
    std::random_device rd;
    std::mt19937 rng{rd()};
};

class HeuristicPlayer : public Player {
public:
    struct Weights {
        double emptyTiles;    
        double monotonicity;   
        double smoothness;     
        double cornerPlacement;
        
        Weights() : emptyTiles(0.2), monotonicity(0.4), 
                   smoothness(0.1), cornerPlacement(0.3) {}
    };

    explicit HeuristicPlayer(const Weights& w = Weights());
    std::pair<int, uint64_t> chooseAction(uint64_t state) override;
    std::string getName() const override;

private:
    Weights weights;
    
    double evaluatePosition(uint64_t state) const;
    double evaluateMonotonicity(uint64_t state) const;
    double evaluateSmoothness(uint64_t state) const;
    double evaluateCornerPlacement(uint64_t state) const;
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
    std::pair<int, uint64_t> chooseAction(uint64_t state) override;
    std::string getName() const override;
};

// Optional: Implement DQN player later
class DQNPlayer : public Player {
public:
    explicit DQNPlayer(const std::string& model_path);
    std::pair<int, uint64_t> chooseAction(uint64_t state) override;
    std::string getName() const override { return "DQN"; }
private:
    // This is a placeholder for the DQN model
    // You'll need to integrate with your preferred ML framework
    void* model;  // Replace with actual ML model type
}; 