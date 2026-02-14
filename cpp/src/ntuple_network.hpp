// ntuple_network.hpp
#pragma once

#include "board.hpp"
#include <vector>
#include <cstddef>
#include <fstream>
#include <string>
#include <cmath>

namespace NTuple {

/** Tile at position pos (0-15) from 64-bit board state. */
inline int getTile(BoardState state, int pos) {
    return static_cast<int>((state >> (pos * 4)) & 0xF);
}

/** Apply transpose to a 64-bit board (same layout as Board). */
inline BoardState transformTranspose(BoardState state) {
    BoardState a1 = state & 0xF0F00F0FF0F00F0FULL;
    BoardState a2 = state & 0x0000F0F00000F0F0ULL;
    BoardState a3 = state & 0x0F0F00000F0F0000ULL;
    BoardState a = a1 | (a2 << 12) | (a3 >> 12);
    BoardState b1 = a & 0xFF00FF0000FF00FFULL;
    BoardState b2 = a & 0x00FF00FF00000000ULL;
    BoardState b3 = a & 0x00000000FF00FF00ULL;
    return b1 | (b2 >> 24) | (b3 << 24);
}

/** Mirror horizontally (exchange columns). */
inline BoardState transformMirror(BoardState state) {
    BoardState out = 0;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            int srcPos = r * 4 + (3 - c);
            int dstPos = r * 4 + c;
            int tile = getTile(state, srcPos);
            out |= (static_cast<BoardState>(tile) << (dstPos * 4));
        }
    }
    return out;
}

/** Flip vertically (exchange rows). */
inline BoardState transformFlip(BoardState state) {
    BoardState out = 0;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            int srcPos = (3 - r) * 4 + c;
            int dstPos = r * 4 + c;
            int tile = getTile(state, srcPos);
            out |= (static_cast<BoardState>(tile) << (dstPos * 4));
        }
    }
    return out;
}

/** Rotate clockwise: transpose then mirror. */
inline BoardState transformRotateClockwise(BoardState state) {
    return transformMirror(transformTranspose(state));
}

/** Rotate counterclockwise: transpose then flip. */
inline BoardState transformRotateCounterclockwise(BoardState state) {
    return transformFlip(transformTranspose(state));
}

/** Index board: at position i the value is i. Used to compute isomorphic mappings. */
constexpr BoardState kIndexBoard = 0xFEDCBA9876543210ULL;

/** Build the 8 symmetric index boards (4 rotations, then same 4 with mirror). */
inline void getSymmetricIndexBoards(BoardState out[8]) {
    BoardState idx = kIndexBoard;
    for (int i = 0; i < 4; ++i) {
        out[i] = idx;
        idx = transformRotateClockwise(idx);
    }
    idx = transformMirror(kIndexBoard);
    for (int i = 4; i < 8; ++i) {
        out[i] = idx;
        idx = transformRotateClockwise(idx);
    }
}

/**
 * A single n-tuple pattern: a fixed set of positions, 8-way isomorphic lookups,
 * and a weight table of size 16^tupleSize.
 */
class NTuplePattern {
public:
    /** pattern: positions 0-15 to sample (e.g. {0,1,2,3,4,5}). iso_count is 8. */
    NTuplePattern(std::vector<int> pattern, int isoCount = 8)
        : pattern_(std::move(pattern))
        , isoCount_(isoCount)
        , tableSize_(static_cast<size_t>(std::pow(16.0, static_cast<double>(pattern_.size()))))
        , weights_(tableSize_, 0.0f)
    {
        if (pattern_.empty())
            return;
        BoardState symBoards[8];
        getSymmetricIndexBoards(symBoards);
        isom_.resize(static_cast<size_t>(isoCount));
        for (int i = 0; i < isoCount; ++i) {
            isom_[i].resize(pattern_.size());
            for (size_t j = 0; j < pattern_.size(); ++j) {
                isom_[i][j] = getTile(symBoards[i], pattern_[j]);
            }
        }
    }

    size_t getTableSize() const { return tableSize_; }
    size_t getPatternLength() const { return pattern_.size(); }

    /** Compute index for one isomorphic view: read tiles at isom[sym][0..len-1] from state. */
    size_t indexOf(const std::vector<int>& isomRow, BoardState state) const {
        size_t index = 0;
        for (size_t i = 0; i < isomRow.size(); ++i)
            index |= (static_cast<size_t>(getTile(state, isomRow[i])) << (4 * i));
        return index;
    }

    /** Sum of weights over all 8 isomorphic lookups. */
    float estimate(BoardState state) const {
        float value = 0.0f;
        for (int i = 0; i < isoCount_; ++i) {
            size_t idx = indexOf(isom_[static_cast<size_t>(i)], state);
            value += weights_[idx];
        }
        return value;
    }

    /** TD update: add (adjust) to each isomorphic lookup; return new sum. */
    float update(BoardState state, float adjust) {
        float value = 0.0f;
        float a = adjust / static_cast<float>(isoCount_);
        for (int i = 0; i < isoCount_; ++i) {
            size_t idx = indexOf(isom_[static_cast<size_t>(i)], state);
            weights_[idx] += a;
            value += weights_[idx];
        }
        return value;
    }

    std::vector<float>& getWeights() { return weights_; }
    const std::vector<float>& getWeights() const { return weights_; }

    void save(std::ostream& out) const {
        size_t len = pattern_.size();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(reinterpret_cast<const char*>(pattern_.data()), sizeof(int) * len);
        out.write(reinterpret_cast<const char*>(weights_.data()), sizeof(float) * tableSize_);
    }

    void load(std::istream& in) {
        size_t len = 0;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len != pattern_.size())
            return;
        in.ignore(sizeof(int) * len);
        in.read(reinterpret_cast<char*>(weights_.data()), sizeof(float) * tableSize_);
    }

private:
    std::vector<int> pattern_;
    int isoCount_;
    size_t tableSize_;
    std::vector<float> weights_;
    std::vector<std::vector<int>> isom_;
};

/**
 * Network of several n-tuple patterns; default 4x6-tuple from the literature.
 */
class NTupleNetwork {
public:
    NTupleNetwork() {
        addDefaultPatterns();
    }

    void addDefaultPatterns() {
        patterns_.clear();
        patterns_.emplace_back(std::vector<int>{0, 1, 2, 3, 4, 5});
        patterns_.emplace_back(std::vector<int>{4, 5, 6, 7, 8, 9});
        patterns_.emplace_back(std::vector<int>{0, 1, 2, 4, 5, 6});
        patterns_.emplace_back(std::vector<int>{4, 5, 6, 8, 9, 10});
    }

    float estimate(BoardState state) const {
        float value = 0.0f;
        for (const auto& p : patterns_)
            value += p.estimate(state);
        return value;
    }

    float update(BoardState state, float adjust) {
        float value = 0.0f;
        float a = adjust / static_cast<float>(patterns_.size());
        for (auto& p : patterns_)
            value += p.update(state, a);
        return value;
    }

    void save(const std::string& path) const {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out)
            return;
        size_t n = patterns_.size();
        out.write(reinterpret_cast<const char*>(&n), sizeof(n));
        for (const auto& p : patterns_)
            p.save(out);
    }

    void load(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in)
            return;
        size_t n = 0;
        in.read(reinterpret_cast<char*>(&n), sizeof(n));
        if (n != patterns_.size())
            return;
        for (auto& p : patterns_)
            p.load(in);
    }

    std::vector<NTuplePattern>& getPatterns() { return patterns_; }
    const std::vector<NTuplePattern>& getPatterns() const { return patterns_; }

private:
    std::vector<NTuplePattern> patterns_;
};

} // namespace NTuple
