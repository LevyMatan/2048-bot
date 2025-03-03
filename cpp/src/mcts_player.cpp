#include "player.hpp"
#include "board.hpp"
#include <cmath>
#include <algorithm>
#include <limits>
#include <random>

MCTSNode::MCTSNode(uint64_t s, MCTSNode* p, bool chance)
    : state(s), visits(0), totalScore(0), parent(p), isChanceNode(chance) {}

double MCTSNode::UCB1(double C) const {
    if (visits == 0) return std::numeric_limits<double>::infinity();
    double exploitation = totalScore / visits;
    double exploration = C * std::sqrt(std::log(parent->visits) / visits);
    return exploitation + exploration;
}

MCTSPlayer::MCTSPlayer(int sims) 
    : simulations(sims), rng(std::random_device{}()) {}

MCTSNode* MCTSPlayer::select(MCTSNode* node) {
    while (!node->children.empty()) {
        if (node->isChanceNode) {
            // For chance nodes, randomly select a child
            std::uniform_int_distribution<> dist(0, static_cast<int>(node->children.size()) - 1);
            node = node->children[dist(rng)].get();
        } else {
            // For decision nodes, use UCB1
            node = std::max_element(
                node->children.begin(),
                node->children.end(),
                [](const auto& a, const auto& b) {
                    return a->UCB1() < b->UCB1();
                }
            )->get();
        }
    }
    return node;
}

void MCTSPlayer::expand(MCTSNode* node) {
    auto validActions = Board::getValidMoveActions(node->state);
    if (validActions.empty()) {
        return;
    }

    if (!node->isChanceNode) {
        // Expand decision node with all possible moves
        for (const auto& [action, nextState] : validActions) {
            auto child = std::make_unique<MCTSNode>(nextState, node, true);
            node->children.push_back(std::move(child));
        }
    } else {
        // Expand chance node with possible new tile positions
        auto emptyTiles = Board::getEmptyTiles(node->state);
        for (const auto& [row, col] : emptyTiles) {
            // Add both 2 and 4 tiles with their respective probabilities
            for (int value : {1, 2}) {  // 1 represents 2, 2 represents 4
                uint64_t nextState = Board::setTile(node->state, row, col, value);
                auto child = std::make_unique<MCTSNode>(nextState, node, false);
                node->children.push_back(std::move(child));
            }
        }
    }
}

// Helper function to get tile value at position
static int getTileValue(uint64_t state, int row, int col) {
    int shift = (15 - (row * 4 + col)) * 4;
    return (state >> shift) & 0xF;
}

double MCTSPlayer::evaluate(uint64_t state) const {
    double score = 0.0;
    int maxTile = 0;
    int emptyCells = static_cast<int>(Board::getEmptyTiles(state).size());

    // Score based on tile values
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int value = getTileValue(state, row, col);
            if (value > 0) {
                score += (1 << value);
                maxTile = std::max(maxTile, value);
            }
        }
    }

    // Bonus for empty cells (mobility)
    score += emptyCells * 100.0;

    // Bonus for maintaining high values in corners
    int corners[] = {
        getTileValue(state, 0, 0),
        getTileValue(state, 0, 3),
        getTileValue(state, 3, 0),
        getTileValue(state, 3, 3)
    };
    
    for (int corner : corners) {
        if (corner == maxTile) {
            score *= 1.5;
        }
    }

    return score;
}

double MCTSPlayer::simulate(uint64_t state) {
    int moves = 0;
    int maxTile = 0;

    while (moves < 1000) {
        auto validActions = Board::getValidMoveActions(state);
        if (validActions.empty()) break;

        // Make a random move
        std::uniform_int_distribution<> dist(0, static_cast<int>(validActions.size()) - 1);
        int idx = dist(rng);
        auto [action, nextState] = validActions[idx];
        state = nextState;

        // Add a random tile
        auto emptyTiles = Board::getEmptyTiles(state);
        if (!emptyTiles.empty()) {
            std::uniform_int_distribution<> posDist(0, static_cast<int>(emptyTiles.size()) - 1);
            auto [row, col] = emptyTiles[posDist(rng)];
            std::uniform_real_distribution<> valueDist(0, 1);
            int value = valueDist(rng) < 0.9 ? 1 : 2;  // 90% chance for 2, 10% chance for 4
            state = Board::setTile(state, row, col, value);
        }

        moves++;
        
        // Update max tile
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                maxTile = std::max(maxTile, getTileValue(state, row, col));
            }
        }
    }

    // Score based on max tile and number of moves
    return std::pow(2, maxTile) + moves;
}

void MCTSPlayer::backpropagate(MCTSNode* node, double score) {
    while (node != nullptr) {
        node->visits++;
        node->totalScore += score;
        node = node->parent;
    }
}

double MCTSPlayer::getExplorationConstant(int depth) const {
    return 1.41 * std::pow(0.95, depth);
}

std::pair<int, uint64_t> MCTSPlayer::chooseAction(uint64_t state) {
    MCTSNode root(state);
    root.visits = 1;  // Initialize root visits to 1

    // Run MCTS for the specified number of simulations
    for (int i = 0; i < simulations; ++i) {
        MCTSNode* node = select(&root);
        expand(node);
        double score = simulate(node->state);
        backpropagate(node, score);
    }

    // Choose the best child based on visit count
    if (root.children.empty()) {
        return {-1, state};
    }

    auto bestChild = std::max_element(
        root.children.begin(),
        root.children.end(),
        [](const auto& a, const auto& b) {
            return a->visits < b->visits;
        }
    );

    int bestAction = static_cast<int>(std::distance(root.children.begin(), bestChild));
    return {bestAction, (*bestChild)->state};
}

std::string MCTSPlayer::getName() const {
    return "MCTS";
} 