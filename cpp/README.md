# 2048 Bot - C++ Implementation

This directory contains the C++ implementation of the 2048 game bot, optimized for performance and efficiency.

## Features

- **Multiple AI Strategies:**  
  Choose from several player types including:
  - `RandomPlayer`: Makes random moves.
  - `MaxEmptyCellsPlayer`: Selects moves that maximize empty cells.
  - `MinMaxPlayer`: Uses a simple min/max heuristic based on board values.
  - `HeuristicPlayer`: Applies a multi-factor heuristic considering empty cells, monotonicity, smoothness, and max tiles.
  - `MonteCarloPlayer`: Simulates random playouts to choose the best statistically averaged move.
  - `ExpectimaxPlayer`: Uses expectimax search with adjustable depth and weighted heuristics.

- **Performance Optimizations:**  
  The C++ implementation includes several optimizations:
  - Bit manipulation for board operations
  - Cache-friendly data structures
  - Move lookup tables
  - Minimal memory allocations during gameplay
  - Precomputed move tables
  - Efficient board state representation

## Directory Structure

```bash
cpp/
├── src/                 # Source code
│   ├── board.h          # Board representation and operations
│   ├── board.cpp        # Implementation of board operations
│   ├── game.h           # Game logic and flow control
│   ├── game.cpp         # Game logic implementation
│   ├── players/         # Player strategy implementations
│   └── main.cpp         # Entry point
├── build/               # Build directory (generated)
└── CMakeLists.txt       # CMake configuration
```

## Setup Instructions

1. **Prerequisites:**
   - CMake (3.31 or higher)
   - C++ compiler supporting C++17
   - Git

2. **Build the project:**
   ```bash
   cd cpp
   mkdir -p build
   cd build
   cmake ..
   cmake --build .
   ```

## Running the Bot

Run the game using the following command from the build directory:

```bash
./2048bot [options]
```

**Command-line options:**
- `-n, --num-games`: Number of games to play (default: 1000)
- `-p, --player`: Player type (random, maxempty, minmax, heuristic, montecarlo, expectimax)
- `-d, --debug`: Enable debug output
- `-b, --benchmark`: Run performance benchmarks

## How It Works

1. **Game Initialization:**  
   The game board is initialized with a compact representation optimized for C++.

2. **AI Decision Making:**  
   Each player type implements the `Player` interface, providing its own move selection strategy.

3. **Board Management:**  
   - Efficient board operations using bitwise operations
   - Move simulation using precomputed lookup tables
   - Optimized random tile generation

4. **Game Loop:**  
   - Move validation
   - State updates
   - Score tracking
   - Performance monitoring

## Customization

- **AI Strategy Tuning:**  
  Modify parameters in the player implementations to experiment with different strategies.

- **Performance Improvements:**  
  The codebase is designed for easy profiling and optimization.

---

Return to [Main README](../README.md) 