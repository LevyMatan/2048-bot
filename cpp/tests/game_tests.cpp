#include "game.hpp"
#include "board.hpp"
#include "players.hpp"
#include "logger.hpp"
#include "score_types.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <tuple>
#include <memory>
#include "test_helpers.hpp"

Logger2048::Logger &logger = Logger2048::Logger::getInstance();

// Mock player for testing
class MockPlayer : public Player {
public:
    MockPlayer(int fixedAction) : fixedAction_(fixedAction) {}

    ChosenActionResult chooseAction(uint64_t state) override {
        // For testing purposes, we'll simulate a simple move
        auto validMoves = Board::getValidMoveActionsWithScores(state);
        if (validMoves.empty()) {
            return {Action::INVALID, state, 0};
        }

        // Find the action that matches our fixed action, or use the first valid move
        for (const auto& [action, nextState, moveScore] : validMoves) {
            if (action == static_cast<Action>(fixedAction_)) {
                return {action, nextState, moveScore};
            }
        }

        // If our fixed action isn't valid, use the first valid move
        auto [action, nextState, moveScore] = validMoves[0];
        return {action, nextState, moveScore};
    }

    std::string getName() const override {
        return "MockPlayer";
    }

private:
    int fixedAction_;
};

// Test fixture for Game class
class GameTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Disable waiting in tests
        waitDisabler = std::make_unique<TestHelpers::ScopedWaitDisabler>();

        // Set up logger for testing
        Logger2048::LoggerConfig testConfig;
        testConfig.level = Logger2048::Level::Debug;
        testConfig.waitEnabled = false;
        logger.configure(testConfig);

        game = new Game2048();

        // Create a mock player that always tries to move left (0)
        mockPlayer = new MockPlayer(0);
    }

    void TearDown() override {
        waitDisabler.reset();
        delete game;
        delete mockPlayer;
    }

    // Helper function to create a test player that always makes a specific move
    std::function<ChosenActionResult(BoardState)> createTestPlayer(Action fixedAction) {
        return [fixedAction](BoardState state) -> ChosenActionResult {
            auto validMoves = Board::getValidMoveActionsWithScores(state);

            // Look for the specified action
            for (const auto& move : validMoves) {
                if (move.action == fixedAction) {
                    return move;
                }
            }

            // If the specific action isn't valid, return the first valid move
            if (!validMoves.empty()) {
                return validMoves[0];
            }

            // If no valid moves, return invalid action
            return {Action::INVALID, state, 0};
        };
    }

    Game2048* game;
    MockPlayer* mockPlayer;

private:
    std::unique_ptr<TestHelpers::ScopedWaitDisabler> waitDisabler;
};

// Test game initialization
TEST_F(GameTest, InitializationTest) {
    // A new game should have a non-zero state (due to initial tiles)
    EXPECT_NE(game->getState(), 0);

    // Score and move count should be 0
    EXPECT_EQ(game->getScore(), 0);
    EXPECT_EQ(game->getMoveCount(), 0);
}

// Test reset functionality
TEST_F(GameTest, ResetTest) {
    // Play some moves to change the game state
    game->playMove(Action::LEFT, 0x12345678, 100);
    game->playMove(Action::UP, 0x87654321, 200);

    // Verify that the state, score, and move count have changed
    EXPECT_NE(game->getState(), 0);
    EXPECT_EQ(game->getScore(), 300);
    EXPECT_EQ(game->getMoveCount(), 2);

    // Reset the game
    game->reset();

    // Verify that score and move count are reset
    EXPECT_EQ(game->getScore(), 0);
    EXPECT_EQ(game->getMoveCount(), 0);

    // State should be non-zero (due to initial tiles)
    EXPECT_NE(game->getState(), 0);
}

// Test playing a single move
TEST_F(GameTest, PlayMoveTest) {
    // Get the initial state
    BoardState initialState = game->getState();

    // Get valid moves for the initial state
    auto validMoves = game->getValidMoves();

    // If there are valid moves, play one and check that the state changes
    if (!validMoves.empty()) {
        ChosenActionResult move = validMoves[0];
        bool success = game->playMove(move.action, move.state, move.score);

        EXPECT_TRUE(success);
        EXPECT_NE(game->getState(), initialState);
        EXPECT_GT(game->getMoveCount(), 0);
        EXPECT_EQ(game->getScore(), move.score);
    }
}

// Test playing a full game
TEST_F(GameTest, PlayFullGameTest) {
    // Create a player that always tries to move left, then up, then right, then down
    auto testPlayer = [](BoardState state) -> ChosenActionResult {
        auto validMoves = Board::getValidMoveActionsWithScores(state);
        if (validMoves.empty()) {
            return {Action::INVALID, state, 0};
        }
        return validMoves[0];
    };

    // Play the game (use 0 for the initial state to start a new game)
    auto [moveCount, finalState, finalScore] = game->playGame(testPlayer, 0);

    // The game should end eventually
    EXPECT_GE(moveCount, 1);
    EXPECT_GE(finalScore, 0);

    // For a new test game, try with a specific initial state
    BoardState initialState = 0x0000000100020003; // Some test initial state
    Game2048 testGame;
    auto [testMoves, testState, testScore] = testGame.playGame(testPlayer, initialState);

    // Verify game started with our initial state and then changed
    EXPECT_NE(testState, initialState);
    EXPECT_GE(testMoves, 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}