#include "gtest/gtest.h"
#include "board.hpp"
#include "game.hpp"
#include "players.hpp"
#include "evaluation.hpp"
#include "logger.hpp"
#include <random>

Logger2048::Logger &logger = Logger2048::Logger::getInstance();

TEST(PlayersTest, ExpectimaxDepth0EqualsHeuristic) {
    // Create identical evaluation parameters
    Evaluation::EvalParams params;
    params["emptyTiles"] = 270.0;
    params["monotonicity"] = -47.0;
    params["mergeability"] = 700.0;
    params["coreScore"] = -11.0;
    
    // Initialize players with same parameters
    HeuristicPlayer heuristicPlayer(params);
    
    // ExpectimaxPlayer constructor params: depth, chanceCovering, timeLimit, adaptive_depth, params
    ExpectimaxPlayer expectimaxPlayer(0, 1, 50.0, false, params);
    
    // Test multiple board states
    for (int seed = 0; seed < 1000; seed++) {
        // Generate a random uint64_t
        BoardState state = rand();
        
        // Get actions from both players
        ChosenActionResult hAction = heuristicPlayer.chooseAction(state);
        ChosenActionResult eAction = expectimaxPlayer.chooseAction(state);
        
        EXPECT_EQ(hAction.action, eAction.action) << "Moves differ for seed " << seed;
        EXPECT_EQ(hAction.state, eAction.state) << "States differ for seed " << seed;
        EXPECT_EQ(hAction.score, eAction.score) << "Scores differ for seed " << seed;
    }
} 