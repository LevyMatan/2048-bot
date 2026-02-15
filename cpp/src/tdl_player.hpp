// tdl_player.hpp
#pragma once

#include "board.hpp"
#include "players.hpp"
#include "ntuple_network.hpp"
#include <memory>
#include <random>
#include <string>
#include <vector>

/**
 * TDL Player: selects moves by maximizing (reward + V(afterstate)) where V is
 * the n-tuple network evaluation. Supports loading pre-trained weights and
 * a standalone training loop with TD(0).
 */
class TDLPlayer : public Player {
public:
    /** Use an existing network (e.g. loaded from file). */
    explicit TDLPlayer(std::shared_ptr<NTuple::NTupleNetwork> network);

    /** Create with a new network (for training); optionally load from path. */
    explicit TDLPlayer(const std::string& weightsPath = "");

    ChosenActionResult chooseAction(BoardState state) override;
    std::string getName() const override { return "TDL"; }

    NTuple::NTupleNetwork& getNetwork() { return *network_; }
    const NTuple::NTupleNetwork& getNetwork() const { return *network_; }

    /**
     * Train the network by self-play with TD(0).
     * @param network Shared network (all threads read/write the same weights)
     * @param episodes Total number of games to play across all threads
     * @param alpha Learning rate
     * @param savePath Path to save weights (optional; can be empty)
     * @param statsInterval Print stats every N games (0 = end only)
     * @param numThreads Number of parallel training threads (default: 1)
     *
     * When numThreads > 1, uses Hogwild-style asynchronous updates: each thread
     * plays its own games and updates the shared weight tables without locks.
     * This is safe because individual float updates are small (alpha * error / 32)
     * and the stochastic nature of TD learning tolerates minor races. In practice,
     * Hogwild converges as well as sequential training and scales nearly linearly.
     */
    static void trainNetwork(std::shared_ptr<NTuple::NTupleNetwork> network,
                            int episodes,
                            float alpha,
                            const std::string& savePath,
                            int statsInterval = 1000,
                            int numThreads = 1);

private:
    std::shared_ptr<NTuple::NTupleNetwork> network_;
    std::mt19937 rng_;
};
