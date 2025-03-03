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
        // For testing purposes, we'll simulate a simple move
        // In a real scenario, we would use Board::getValidMoveActions
        auto validMoves = Board::getValidMoveActions(state);
        if (validMoves.empty()) {
            return {-1, state};
        }
        
        // Find the action that matches our fixed action, or use the first valid move
        for (const auto& [action, nextState] : validMoves) {
            if (static_cast<int>(action) == fixedAction_) {
                return {fixedAction_, nextState};
            }
        }
        
        // If our fixed action isn't valid, use the first valid move
        auto [action, nextState] = validMoves[0];
        return {static_cast<int>(action), nextState};
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
        // Create a game
        game = new Game2048();
        
        // Create a mock player that always tries to move left (0)
        mockPlayer = new MockPlayer(0);
    }
    
    void TearDown() override {
        delete game;
        delete mockPlayer;
    }
    
    Game2048* game;
    MockPlayer* mockPlayer;
    
    // Helper function to play a move using the mock player
    bool playMoveWithMockPlayer() {
        auto [action, nextState] = mockPlayer->chooseAction(game->getState());
        return game->playMove(action, nextState);
    }
};

// Test game initialization
TEST_F(GameTest, InitializationTest) {
    // A new game should have a non-zero board state (initial tiles)
    EXPECT_NE(game->getState(), 0);
    
    // Initial score should be 0
    EXPECT_EQ(game->getScore(), 0);
}

// Test game step
TEST_F(GameTest, StepTest) {
    // Save initial state
    uint64_t initialState = game->getState();
    
    // Perform a step
    bool validMove = playMoveWithMockPlayer();
    
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
    playMoveWithMockPlayer();
    playMoveWithMockPlayer();
    
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
    
    while (moveCount < maxMoves && playMoveWithMockPlayer()) {
        moveCount++;
    }
    
    // Game should have a score and state
    EXPECT_GE(game->getScore(), 0);
    EXPECT_NE(game->getState(), 0);
    EXPECT_GT(game->getMoveCount(), 0);
}

// Test playing a full game
TEST_F(GameTest, PlayFullGameTest) {
    // Create a lambda that captures the mock player and calls its chooseAction method
    auto chooseActionFn = [this](uint64_t state) {
        return mockPlayer->chooseAction(state);
    };
    
    // Play a full game
    auto [score, state, moves] = game->playGame(chooseActionFn);
    
    // Game should have completed
    EXPECT_GE(score, 0);
    EXPECT_NE(state, 0);
    EXPECT_GT(moves, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 