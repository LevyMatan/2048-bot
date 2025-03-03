# 2048 Bot

A bot to play the 2048 game using various AI strategies. The project explores different decision-making algorithms to achieve high scores, with some algorithms capable of reaching scores of 20,000 and beyond.

This project contains both C++ and Python implementations of the 2048 game bot, each with different optimization strategies and features.

## Project Structure

```bash
2048-bot
├── cpp/                    # C++ implementation
│   ├── src/                # C++ source files
│   ├── build/              # Build directory
│   ├── CMakeLists.txt      # CMake configuration
│   └── README.md           # C++ implementation details
├── python/                 # Python implementation
│   ├── src/                # Python source files
│   ├── test/               # Python tests
│   └── README.md           # Python implementation details
├── LICENSE                 # MIT License
└── README.md               # This file
```

## Implementations

### [C++ Implementation](cpp/README.md)

The C++ implementation focuses on performance optimization and is suitable for running large numbers of games or deep search algorithms. It leverages:

- Bitwise operations for efficient board representation
- Precomputed move tables
- Cache-friendly data structures
- Memory optimization techniques
- Heuristic weight tuning using evolutionary algorithms

### [Python Implementation](python/README.md)

The Python implementation prioritizes readability and ease of experimentation, making it ideal for prototyping new algorithms and strategies. It features:

- Clear, object-oriented design
- Simplified API for creating new player strategies
- Visualization tools
- Comprehensive test suite

## Features

- **Multiple AI Strategies:**  
  Both implementations offer various player types including:
  - Random players
  - Heuristic-based players (empty cells, monotonicity, smoothness, corner placement)
  - Monte Carlo Tree Search players

- **Modular Design:**  
  The project uses clearly separated modules for the game logic, board management, and player strategies, allowing easy experimentation.

- **Heuristic Weight Tuning:**
  The C++ implementation includes a dedicated program for tuning the weights used by the heuristic player. This uses an evolutionary algorithm to find optimal weight combinations that maximize game scores.

## Getting Started

See the README files in the respective implementation directories for specific setup and usage instructions:

- [C++ Setup and Usage](cpp/README.md)
- [Python Setup and Usage](python/README.md)

## License

[MIT License](LICENSE)

---

Happy coding, and good luck reaching those high scores! 🚀
