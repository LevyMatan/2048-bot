#include "player.hpp"
#include "board.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

MCTSNode::MCTSNode(uint64_t s, MCTSNode* p, bool chance)
    : state(s), visits(0), totalScore(0), parent(p), isChanceNode(chance) {}

double MCTSNode::UCB1(double C) const {
    if (visits == 0) return std::numeric_limits<double>::infinity();
    return (totalScore / visits) + C * std::sqrt(std::log(parent->visits) / visits);
}

MCTSPlayer::MCTSPlayer(int sims) 
    : simulations(sims), rng(std::random_device{}()) {}

MCTSNode* MCTSPlayer::select(MCTSNode* node) {
    while (!node->children.empty()) {
        if (node->isChanceNode) {
            // For chance nodes, randomly select a child
            int idx = std::uniform_int_distribution<>(0, node->children.size()-1)(rng);
            node = node->children[idx].get();
        } else {
            // For decision nodes, use UCB1
            node = std::max_element(node->children.begin(), node->children.end(),
                [](const auto& a, const auto& b) {
                    return a->UCB1() < b->UCB1();
                })->get();
        }
    }
    return node;
}

void MCTSPlayer::expand(MCTSNode* node) {
    if (node->isChanceNode) {
        auto emptyTiles = Board::getEmptyTiles(node->state);
        for (auto [row, col] : emptyTiles) {
            // Add both possible new tile values (2 and 4)
            for (int value : {1, 2}) {
                uint64_t newState = Board::setTile(node->state, row, col, value);
                node->children.push_back(
                    std::make_unique<MCTSNode>(newState, node, false));
            }
        }
    } else {
        auto actions = Board::getValidMoveActions(node->state);
        for (const auto& [action, nextState] : actions) {
            node->children.push_back(
                std::make_unique<MCTSNode>(nextState, node, true));
        }
    }
}

double MCTSPlayer::evaluate(uint64_t state) const {
    double score = 0.0;
    int maxTile = 0;
    int emptyCells = Board::getEmptyTiles(state).size();

    // Score based on tile values
    for (int i = 0; i < 16; ++i) {
        int value = (state >> (i * 4)) & 0xF;
        if (value > 0) {
            score += (1 << value);
            maxTile = std::max(maxTile, value);
        }
    }

    // Bonus for empty cells (mobility)
    score += emptyCells * 100.0;

    // Bonus for maintaining high values in corners
    int corners[] = {0, 3, 12, 15};
    for (int pos : corners) {
        int value = (state >> (pos * 4)) & 0xF;
        if (value == maxTile) {
            score *= 1.5;
        }
    }

    return score;
}

double MCTSPlayer::simulate(uint64_t state) {
    double score = 0;
    uint64_t currentState = state;
    int depth = 0;
    const int MAX_DEPTH = 20;
    double discount = 0.95;

    while (depth < MAX_DEPTH) {
        auto actions = Board::getValidMoveActions(currentState);
        if (actions.empty()) break;

        auto bestAction = std::max_element(actions.begin(), actions.end(),
            [this](const auto& a, const auto& b) {
                return evaluate(std::get<1>(a)) < evaluate(std::get<1>(b));
            });

        auto [action, nextState] = *bestAction;

        auto emptyTiles = Board::getEmptyTiles(nextState);
        if (emptyTiles.empty()) break;

        int tileIdx = std::uniform_int_distribution<>(0, emptyTiles.size()-1)(rng);
        auto [row, col] = emptyTiles[tileIdx];
        int value = (std::uniform_real_distribution<>(0, 1)(rng) < 0.9) ? 1 : 2;

        currentState = Board::setTile(nextState, row, col, value);
        double immediateReward = evaluate(currentState);
        score += std::pow(discount, depth) * immediateReward;
        depth++;
    }

    score += std::pow(discount, depth) * evaluate(currentState);
    return score;
}

void MCTSPlayer::backpropagate(MCTSNode* node, double score) {
    while (node) {
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

    for (int i = 0; i < simulations; ++i) {
        MCTSNode* node = select(&root);
        if (node->visits > 0) {
            expand(node);
            if (!node->children.empty()) {
                node = node->children[0].get();
            }
        }
        double score = simulate(node->state);
        backpropagate(node, score);
    }

    auto bestChild = std::max_element(root.children.begin(), root.children.end(),
        [](const auto& a, const auto& b) {
            return a->visits < b->visits;
        });

    return std::make_pair(
        static_cast<int>(std::distance(root.children.begin(), bestChild)),
        (*bestChild)->state
    );
}

std::string MCTSPlayer::getName() const {
    return "Monte Carlo Tree Search Player";
} 