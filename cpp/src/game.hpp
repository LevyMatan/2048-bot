// game.hpp
#pragma once
#include "board.hpp"
#include <random>
#include "player.hpp"

class Game2048 {
private:
    Board board;
    int moveCount;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
    std::unique_ptr<Player> player;

public:
    Game2048()
        : rng(std::random_device{}()),
          dist(0.0, 1.0){
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

    void setPlayer(std::unique_ptr<Player> newPlayer) {
        player = std::move(newPlayer);
    }

    std::string getPlayerName() const { 
        return player ? player->getName() : "No Player"; 
    }

private:

};