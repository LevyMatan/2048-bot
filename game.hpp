// game.hpp
#pragma once
#include "board.hpp"
#include <random>
#include "mcts_player.hpp"

class Game2048 {
private:
    Board board;
    int moveCount;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
    std::unique_ptr<MCTSPlayer> player;

    struct Weights {
        double emptyTiles;
        double monotonicity;
        double smoothness;
        double cornerPlacement;
    };
    static Weights weights;

public:
    Game2048(int mctsSimulations = 1000)
        : rng(std::random_device{}()),
          dist(0.0, 1.0),
          player(std::make_unique<MCTSPlayer>(mctsSimulations)) {
        reset();
    }

    void addRandomTile();
    bool playMove();
    int getScore() const;
    void reset();
    std::tuple<int, uint64_t, int> playGame();
    void prettyPrint() const;

    void setState(uint64_t state) {
        board.setState(state);
    }

    uint64_t getState() const {
        return board.getState();
    }

    static void setWeights(double empty, double mono, double smooth, double corner) {
        weights = {empty, mono, smooth, corner};
    }

private:
    double evaluatePosition(uint64_t state) const;
    double evaluateMonotonicity(uint64_t state) const;
    double evaluateSmoothness(uint64_t state) const;
    double evaluateCornerPlacement(uint64_t state) const;
};