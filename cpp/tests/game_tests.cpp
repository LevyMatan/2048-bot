#include "game.hpp"
#include "player.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <utility>

// Mock player for testing
class MockPlayer : public Player {
public:
    MockPlayer(int fixedAction) : fixedAction_(fixedAction) {}
    
    std::pair<int, uint64_t> chooseAction(uint64_t state) override {
        // Return the fixed action and the state (no simulation)
        return {fixedAction_, state};
    }
    
    std::string getName() const override {
        return "MockPlayer";
    }
    
private:
    int fixedAction_;
};

class GameTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a game with a mock player that always moves left (0)
        game = new Game2048();
        game->setPlayer(std::make_unique<MockPlayer>(0));
    }
    
    void TearDown() override {
        delete game;
    }
    
    Game2048* game;
};

// Test game initialization
TEST_F(GameTest, InitializationTest) {
    // A new game should have a non-zero board state (initial tiles)
    EXPECT_NE(game->getState(), 0);
    
    // Initial score should be 0
    EXPECT_EQ(game->getScore(), 0);
    
    // Game should have a player
    EXPECT_EQ(game->getPlayerName(), "MockPlayer");
}

// Test game step
TEST_F(GameTest, StepTest) {
    // Save initial state
    uint64_t initialState = game->getState();
    
    // Perform a step
    bool validMove = game->playMove();
    
    // The move should be valid or invalid depending on the board state
    // We can't assert a specific outcome since the initial board is random
    
    // If the move was valid, the board state should have changed
    if (validMove) {
        EXPECT_NE(game->getState(), initialState);
    }
}

// Test game reset
TEST_F(GameTest, ResetTest) {
    // Play a few moves
    game->playMove();
    game->playMove();
    
    // Record the state and score
    uint64_t stateBeforeReset = game->getState();
    
    // Reset the game
    game->reset();
    
    // State should be different after reset
    EXPECT_NE(game->getState(), stateBeforeReset);
    
    // Score should be reset to 0
    EXPECT_EQ(game->getScore(), 0);
    
    // Move count should be reset to 0
    EXPECT_EQ(game->getMoveCount(), 0);
}

// Test playing a game with a limited number of moves
TEST_F(GameTest, PlayLimitedMovesTest) {
    // Play a few moves
    int moveCount = 0;
    int maxMoves = 10;
    
    while (moveCount < maxMoves && game->playMove()) {
        moveCount++;
    }
    
    // Game should have a score and state
    EXPECT_GE(game->getScore(), 0);
    EXPECT_NE(game->getState(), 0);
    EXPECT_GT(game->getMoveCount(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 