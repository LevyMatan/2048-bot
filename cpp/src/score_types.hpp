#pragma once

#include <cstdint>

namespace Score {
    // Game score - actual points accumulated during gameplay
    using GameScore = uint64_t;
    
    // Board evaluation score - AI's assessment of board position
    using BoardEval = double;
} 