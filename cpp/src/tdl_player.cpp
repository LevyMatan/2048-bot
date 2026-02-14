// tdl_player.cpp
#include "tdl_player.hpp"
#include "board.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

TDLPlayer::TDLPlayer(std::shared_ptr<NTuple::NTupleNetwork> network)
    : network_(std::move(network))
    , rng_(std::random_device{}())
{}

TDLPlayer::TDLPlayer(const std::string& weightsPath)
    : network_(std::make_shared<NTuple::NTupleNetwork>())
    , rng_(std::random_device{}())
{
    if (!weightsPath.empty())
        network_->load(weightsPath);
}

ChosenActionResult TDLPlayer::chooseAction(BoardState state) {
    auto moves = Board::getValidMoveActionsWithScores(state);
    if (moves.empty())
        return ChosenActionResult(Action::INVALID, state, 0);

    ChosenActionResult best = moves[0];
    float bestVal = static_cast<float>(best.score) + network_->estimate(best.state);

    for (size_t i = 1; i < moves.size(); ++i) {
        const auto& m = moves[i];
        float val = static_cast<float>(m.score) + network_->estimate(m.state);
        if (val > bestVal) {
            bestVal = val;
            best = m;
        }
    }
    return best;
}

namespace {

void addRandomTile(BoardState& state, std::mt19937& rng) {
    auto empty = Board::getEmptyTiles(state);
    if (empty.empty())
        return;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    size_t idx = static_cast<size_t>(dist(rng) * empty.size());
    if (idx >= empty.size())
        idx = empty.size() - 1;
    auto [row, col] = empty[idx];
    int value = (dist(rng) < 0.9f) ? 1 : 2;
    state = Board::setTile(state, row, col, value);
}

} // namespace

void TDLPlayer::trainNetwork(std::shared_ptr<NTuple::NTupleNetwork> network,
                            int episodes,
                            float alpha,
                            const std::string& savePath,
                            int statsInterval) {
    // Ensure Board lookup tables are initialized (they require a Board object
    // to be constructed at least once).
    { Board initBoard; (void)initBoard; }

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    struct Step {
        BoardState afterstate;
        int reward;
    };
    std::vector<Step> path;
    path.reserve(2000);

    std::vector<int> scores;
    scores.reserve(static_cast<size_t>(statsInterval > 0 ? statsInterval : 1));
    int maxScore = 0;
    int maxTileCount[16] = {0};

    for (int ep = 1; ep <= episodes; ++ep) {
        path.clear();
        BoardState state = 0;
        addRandomTile(state, rng);
        addRandomTile(state, rng);
        int totalScore = 0;

        for (;;) {
            auto moves = Board::getValidMoveActionsWithScores(state);
            if (moves.empty())
                break;

            ChosenActionResult best = moves[0];
            float bestVal = static_cast<float>(best.score) + network->estimate(best.state);
            for (size_t i = 1; i < moves.size(); ++i) {
                const auto& m = moves[i];
                float val = static_cast<float>(m.score) + network->estimate(m.state);
                if (val > bestVal) {
                    bestVal = val;
                    best = m;
                }
            }

            totalScore += best.score;
            path.push_back({best.state, static_cast<int>(best.score)});
            state = best.state;
            addRandomTile(state, rng);
        }

        if (path.empty())
            continue;

        // TD(0) backward update
        float target = 0.0f;
        for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i) {
            float err = target - network->estimate(path[static_cast<size_t>(i)].afterstate);
            float newVal = network->update(path[static_cast<size_t>(i)].afterstate, alpha * err);
            target = static_cast<float>(path[static_cast<size_t>(i)].reward) + newVal;
        }

        scores.push_back(totalScore);
        if (totalScore > maxScore)
            maxScore = totalScore;
        BoardState finalState = path.back().afterstate;
        int maxTile = Board::getMaxTileValue(finalState);
        if (maxTile >= 0 && maxTile < 16)
            maxTileCount[maxTile]++;

        if (statsInterval > 0 && ep % statsInterval == 0) {
            int sum = std::accumulate(scores.begin(), scores.end(), 0);
            double avg = static_cast<double>(sum) / static_cast<double>(scores.size());
            std::cout << ep << "\tavg = " << avg << "\tmax = " << maxScore << "\n";
            for (int t = 1; t <= 15; ++t) {
                int count = 0;
                for (int c = t; c < 16; ++c)
                    count += maxTileCount[c];
                if (count == 0)
                    continue;
                double pct = 100.0 * count / static_cast<double>(scores.size());
                double termPct = 100.0 * maxTileCount[t] / static_cast<double>(scores.size());
                std::cout << "\t" << (1 << t) << "\t" << pct << "%\t(" << termPct << "%)\n";
            }
            scores.clear();
            for (int& c : maxTileCount)
                c = 0;
        }
    }

    if (!savePath.empty())
        network->save(savePath);
}
