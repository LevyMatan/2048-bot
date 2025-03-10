#include "gtest/gtest.h"
#include "board.hpp"
#include "game.hpp"
#include "players.hpp"
#include "evaluation.hpp"
#include "logger.hpp"
#include <random>

using namespace Logger2048;
Logger &logger = Logger::getInstance();

TEST(PlayersTest, ExpectimaxDepth0EqualsHeuristic) {

    LoggerConfig cfg = LoggerConfig();
    cfg.level = Level::Debug;
    for (int i = 0; i < static_cast<size_t>(Group::COUNT); i++) {
        cfg.groupsEnabled[i] = true;
    }
    cfg.outputDestination = LogOutput::Console;
    logger.configure(cfg);
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
    for (int seed = 0; seed < 1; seed++) {
        // Generate a random uint64_t
        BoardState state = rand();
        
        // Get actions from both players
        logger.printBoard(Group::Game, state);
        ChosenActionResult hAction = heuristicPlayer.chooseAction(state);
        logger.info(Group::Parser, "Heuristic action: " + actionToString(hAction.action));
        ChosenActionResult eAction = expectimaxPlayer.chooseAction(state);
        logger.info(Group::Parser, "Expectimax action: " + actionToString(eAction.action));

        EXPECT_EQ(hAction.action, eAction.action) << "Moves differ for seed " << seed;
        EXPECT_EQ(hAction.state, eAction.state) << "States differ for seed " << seed;
        EXPECT_EQ(hAction.score, eAction.score) << "Scores differ for seed " << seed;
    }
} 