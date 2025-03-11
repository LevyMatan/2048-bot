// game.hpp
#pragma once
#include "board.hpp"
#include "players.hpp"
#include "score_types.hpp"
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
    Score::GameScore score;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

public:
    Game2048()
        : rng(std::random_device{}()),
          dist(0.0, 1.0){
        reset();
    }

    void addRandomTile();
    bool playMove(Action action, BoardState nextState, Score::GameScore moveScore);
    Score::GameScore getScore() const { return score; }
    void setScore(Score::GameScore newScore) { score = newScore; }
    int getMoveCount() const { return moveCount; }
    void setMoveCount(int newMoveCount) { moveCount = newMoveCount; }
    void reset();
    std::tuple<int, BoardState, Score::GameScore> playGame(
        std::function<ChosenActionResult(BoardState)> chooseActionFn, 
        BoardState initialState);
    void prettyPrint() const;

    void setState(BoardState state) {
        board.setState(state);
    }

    BoardState getState() const {
        return board.getState();
    }

    // Get valid moves for the current state
    std::vector<ChosenActionResult> getValidMoves() const {
        return Board::getValidMoveActionsWithScores(board.getState());
    }
};