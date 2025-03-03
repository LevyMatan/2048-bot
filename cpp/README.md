# 2048 Bot - C++ Implementation

This directory contains the C++ implementation of the 2048 game bot, optimized for performance and efficiency.

## Features

- **Multiple AI Strategies:**  
  Choose from several player types including:
  - `RandomPlayer`: Makes random moves.
  - `HeuristicPlayer`: Applies a multi-factor heuristic considering empty cells, monotonicity, smoothness, and corner placement.
  - `MCTSPlayer`: Uses Monte Carlo Tree Search to simulate and evaluate possible moves.

- **Performance Optimizations:**  
  The C++ implementation includes several optimizations:
  - Bit manipulation for board operations
  - Cache-friendly data structures
  - Move lookup tables
  - Minimal memory allocations during gameplay
  - Precomputed move tables
  - Efficient board state representation

- **Heuristic Weight Tuning:**
  - Evolutionary algorithm to optimize heuristic weights
  - Customizable population size, generations, and evaluation games
  - Ability to save and load weights from CSV files
  - Support for continuing optimization from previous runs

## Directory Structure

```bash
cpp/
├── src/                 # Source code
│   ├── board.hpp        # Board representation and operations
│   ├── board.cpp        # Implementation of board operations
│   ├── game.hpp         # Game logic and flow control
│   ├── game.cpp         # Game logic implementation
│   ├── player.hpp       # Player interface and implementations
│   ├── random_player.cpp # Random player implementation
│   ├── heuristic_player.cpp # Heuristic player implementation
│   ├── mcts_player.cpp  # Monte Carlo Tree Search player implementation
│   ├── tune_heuristic.cpp # Heuristic weight tuning program
│   └── main.cpp         # Entry point
├── build/               # Build directory (generated)
└── CMakeLists.txt       # CMake configuration
```

## Setup Instructions

1. **Prerequisites:**
   - CMake (3.10 or higher)
   - C++ compiler supporting C++17
   - Git

2. **Build the project:**
   ```bash
   cd cpp
   mkdir -p build
   cd build
   
   # For Debug build (default)
   cmake ..
   cmake --build . --config Debug
   
   # For Release build (optimized)
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

3. **Build with tests:**
   ```bash
   cd cpp
   mkdir -p build
   cd build
   
   # For Debug build with tests
   cmake .. -DBUILD_TESTS=ON
   cmake --build . --config Debug
   
   # For Release build with tests
   cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

## Running the Game

Run the game using the following command from the build directory:

```bash
# For Debug build
./Debug/2048 [player_type] [num_games] [weights_file]

# For Release build (faster execution)
./Release/2048 [player_type] [num_games] [weights_file]
```

**Command-line options:**
- `player_type`: Player type (Random, Heuristic, MCTS)
- `num_games`: Number of games to play (default: 1000)
- `weights_file`: Optional CSV file with custom weights for the Heuristic player

**Examples:**
```bash
# Play 100 games with the Random player (Debug build)
./Debug/2048 Random 100

# Play 50 games with the Heuristic player using default weights (Debug build)
./Debug/2048 Heuristic 50

# Play 25 games with the Heuristic player using custom weights (Debug build)
./Debug/2048 Heuristic 25 best_weights.csv

# Play 10 games with the MCTS player (Debug build)
./Debug/2048 MCTS 10

# Play 1000 games with the Heuristic player (Release build - much faster)
./Release/2048 Heuristic 1000

# Run performance benchmark with 5000 games (Release build recommended)
./Release/2048 Heuristic 5000
```

## Running the Tests

The project includes unit tests for core components using Google Test. To run the tests:

```bash
# Run tests (Debug build)
cd build
ctest -C Debug

# Run specific test (Debug build)
cd build/tests
./Debug/board_tests

# Run tests (Release build - faster execution)
cd build
ctest -C Release

# Run specific test (Release build)
cd build/tests
./Release/board_tests
```

The tests cover various aspects of the `Board` class functionality:
- Board initialization and state management
- Tile value conversions
- Setting and getting tiles
- Move simulation and validation
- Edge cases and random board scenarios

Adding more tests for other components is encouraged to maintain code quality and prevent regressions.

## Heuristic Weight Tuning

The project includes a dedicated program for tuning the weights used by the `HeuristicPlayer`. This program uses an evolutionary algorithm to find optimal weight combinations.

### Running the Tuning Program

```bash
# Debug build
./Debug/tune_heuristic [options]

# Release build (recommended for tuning as it's much faster)
./Release/tune_heuristic [options]
```

**Available options:**
- `--population N`: Set population size (default: 20)
- `--generations N`: Set number of generations (default: 10)
- `--games N`: Set games per evaluation (default: 10)
- `--mutation X`: Set mutation rate (default: 0.1)
- `--elite X`: Set elite percentage (default: 0.2)
- `--output FILE`: Set output file for all weights (default: heuristic_weights.csv)
- `--best-output FILE`: Set output file for best weights (default: best_weights.csv)
- `--continue`: Continue from existing weights file

**Examples:**
```bash
# Run with default parameters (Debug build)
./Debug/tune_heuristic

# Run with custom parameters (Debug build)
./Debug/tune_heuristic --population 30 --generations 15 --games 20

# Continue optimization from previous run (Debug build)
./Debug/tune_heuristic --continue --population 20 --generations 5 --games 10

# Run extensive optimization (Release build recommended)
./Release/tune_heuristic --population 50 --generations 30 --games 50

# Continue optimization with more generations (Release build)
./Release/tune_heuristic --continue --population 50 --generations 20 --games 50
```

### Understanding the Weights

The `HeuristicPlayer` uses four key factors to evaluate board positions:

1. **Empty Tiles (`emptyTiles`)**: Rewards having more empty cells on the board, which provides more flexibility for future moves.

2. **Monotonicity (`monotonicity`)**: Rewards arrangements where tiles increase or decrease monotonically along rows and columns, which helps maintain order.

3. **Smoothness (`smoothness`)**: Rewards arrangements where adjacent tiles have similar values, making it easier to merge them.

4. **Corner Placement (`cornerPlacement`)**: Rewards keeping high-value tiles in the corners, which is a common strategy for achieving high scores.

The weights determine the relative importance of each factor in the overall evaluation function. The tuning program finds the optimal balance between these factors.

### Weight Files Format

The weight files use a simple CSV format:

```
emptyTiles,monotonicity,smoothness,cornerPlacement,avgScore,maxScore,gamesPlayed
0.047740,0.241106,0.326124,0.385031,10374.616000,48188,500
```

The first line is a header, and subsequent lines contain the weight values followed by performance metrics.

## How It Works

1. **Game Initialization:**  
   The game board is initialized with two random tiles (2 or 4).

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
  Use the tuning program to find optimal weights for the `HeuristicPlayer`.

- **Performance Improvements:**  
  The codebase is designed for easy profiling and optimization.

---

Return to [Main README](../README.md) 