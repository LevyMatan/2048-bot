// game.hpp
#pragma once
#include "board.hpp"
#include <random>

class Game2048 {
private:
    Board board;
    int moveCount;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

public:
    Game2048() : moveCount(0), rng(std::random_device{}()), dist(0.0, 1.0) {
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

private:
    double evaluatePosition(uint64_t state) const;
    double evaluateMonotonicity(uint64_t state) const;
    double evaluateSmoothness(uint64_t state) const;
    double evaluateCornerPlacement(uint64_t state) const;
};