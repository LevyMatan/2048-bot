# 2048 Bot - Python Implementation

This directory contains the Python implementation of the 2048 game bot, designed for readability, flexibility, and ease of experimentation.

## Features

- **Multiple AI Strategies:**  
  Choose from several player types including:
  - `RandomPlayer`: Makes random moves.
  - `MaxEmptyCellsPlayer`: Selects moves that maximize empty cells.
  - `MinMaxPlayer`: Uses a simple min/max heuristic based on board values.
  - `HeuristicPlayer`: Applies a multi-factor heuristic considering empty cells, monotonicity, smoothness, and max tiles.

- **Python-Specific Advantages:**  
  - Clear, object-oriented design
  - Easy to read and modify code
  - Simplified API for creating new player strategies
  - Visualization capabilities
  - Comprehensive test suite

## Directory Structure

```bash
python/
├── src/                 # Source code
│   ├── board.py         # Board representation and operations
│   ├── game.py          # Game logic and flow control
│   ├── players.py       # Player strategy implementations
│   └── bot.py           # Main entry point
└── test/                # Test suite
    ├── test_board.py    # Tests for board functionality
    ├── test_game.py     # Tests for game logic
    └── test_players.py  # Tests for player strategies
```

## Setup Instructions

1. **Prerequisites:**
   - Python 3.8 or higher
   - pip (Python package manager)

2. **Installation:**
   ```bash
   # Create and activate a virtual environment (optional but recommended)
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   
   # Install dependencies
   pip install -r requirements.txt
   ```

## Running the Bot

Run the game using the following command:

```bash
python -m src.bot [options]
```

**Command-line options:**
- `--games`: Number of games to play (default: 100)
- `--player`: Player type (random, maxempty, minmax, heuristic)
- `--verbose`: Enable detailed output
- `--visualize`: Show graphical representation of the game

## How It Works

1. **Game Initialization:**  
   A 4x4 grid is initialized with two random tiles (either 2 or 4).

2. **AI Decision Making:**  
   Each player type implements the `Player` class interface, providing its own `get_move()` method.

3. **Board Management:**  
   - The board is represented as a 2D array
   - Move functions handle tile sliding and merging
   - Random tile generation follows the game's probability rules

4. **Game Loop:**  
   - Input handling
   - Move validation and execution
   - Score tracking
   - Game state monitoring

## Running Tests

Tests can be run using pytest:

```bash
pytest test/
```

## Customization

- **Creating Custom Players:**  
  Extend the `Player` class and implement the `get_move()` method to create your own AI strategy.

- **Visualization Customization:**  
  Modify the visualization settings in the configuration to change the appearance of the game.

---

Return to [Main README](../README.md) 