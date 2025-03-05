// game.hpp
#pragma once
#include "board.hpp"
#include <random>
#include <tuple>
#include <iostream>
#include <vector>
#include <functional>

// Forward declaration of Action enum
enum class Action;

class Game2048 {
private:
    Board board;
    int moveCount;
    int score;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

public:
    Game2048()
        : rng(std::random_device{}()),
          dist(0.0, 1.0){
        reset();
    }

    void addRandomTile();
    bool playMove(Action action, uint64_t nextState, int moveScore);
    int getScore() const { return score; }
    void setScore(int newScore) { score = newScore; }
    int getMoveCount() const { return moveCount; }
    void setMoveCount(int newMoveCount) { moveCount = newMoveCount; }
    void reset();
    std::tuple<int, uint64_t, int> playGame(std::function<std::tuple<Action, uint64_t, int>(uint64_t)> chooseActionFn);
    void prettyPrint() const;

    void setState(uint64_t state) {
        board.setState(state);
    }

    uint64_t getState() const {
        return board.getState();
    }

    // Get valid moves for the current state
    std::vector<std::tuple<Action, uint64_t, int>> getValidMoves() const {
        return Board::getValidMoveActionsWithScores(board.getState());
    }
};