# 2048 Bot

A bot to play the 2048 game using various AI strategies implemented in C++. The project explores different decision-making algorithms to achieve high scores, with some algorithms capable of reaching scores of 20,000 and beyond.

## Features

- **Multiple AI Strategies:**  
  Choose from several player types including:
  - `RandomPlayer`: Makes random moves.
  - `MaxEmptyCellsPlayer`: Selects moves that maximize empty cells.
  - `MinMaxPlayer`: Uses a simple min/max heuristic based on board values.
  - `HeuristicPlayer`: Applies a multi-factor heuristic considering empty cells, monotonicity, smoothness, and max tiles.
  - `MonteCarloPlayer`: Simulates random playouts to choose the best statistically averaged move.
  - `ExpectimaxPlayer`: Uses expectimax search with adjustable depth and weighted heuristics.

- **Optimized C++ Implementation:**  
  The codebase leverages C++'s performance benefits and includes optimizations like:
  - Precomputed move tables
  - Efficient board state representation
  - Memory-optimized data structures

- **Modular Design:**  
  The project uses clearly separated modules for the game logic, board management, and player strategies, allowing easy experimentation.

## Project Structure

```bash
2048-bot
├── src
│   ├── board.py             # Contains game board representation, move simulation, and helper functions.
│   ├── game.py              # Main game loop and integration of player strategies.
│   ├── players.py           # Definitions for various player strategies.
│   └── bot.py               # (Optional) An alternative entry point for playing the game.
├── requirements.txt         # Python package dependencies.
└── README.md                # This file.
```

## Setup Instructions

1. **Prerequisites:**
   - CMake (3.31 or higher)
   - C++ compiler supporting C++17
   - Git

2. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd 2048-bot
   ```

3. **Build the project:**
   ```bash
   mkdir build
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

## Performance Optimization

The C++ implementation includes several optimizations:

- Bit manipulation for board operations
- Cache-friendly data structures
- Move lookup tables
- Minimal memory allocations during gameplay

## Customization & Contribution

- **AI Strategy Tuning:**  
  Modify parameters in the player implementations to experiment with different strategies.

- **Performance Improvements:**  
  The codebase is designed for easy profiling and optimization.

- **Contributing:**  
  1. Fork the repository
  2. Create a feature branch
  3. Submit a pull request with your changes

## License

[MIT License](LICENSE)

---

Happy coding, and good luck reaching those high scores! 🚀
