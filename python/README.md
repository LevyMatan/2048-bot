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
  - Detailed benchmarking with HTML reports

## Directory Structure

```bash
python/
├── src/
│   └── game2048/           # Main package
│       ├── __init__.py
│       ├── board.py        # Board representation and operations
│       ├── game.py         # Game logic and flow control
│       ├── players.py      # Player strategy implementations
│       ├── interfaces.py   # Game interfaces (CLI, GYM)
│       ├── benchmark.py    # Benchmarking functionality
│       └── main.py         # Main entry point
├── tests/                  # Test suite
│   ├── __init__.py
│   ├── test_board.py      # Tests for board functionality
│   ├── test_game.py       # Tests for game logic
│   └── test_benchmark.py  # Tests for benchmark functionality
├── examples/              # Example scripts and usage
├── htmlcov/              # Coverage reports
├── setup.py              # Package setup file
├── setup.bat            # Windows setup script
├── pyproject.toml       # Project configuration
└── README.md           # This file
```

## Setup Instructions

1. **Prerequisites:**
   - Python 3.10 or higher
   - pip (Python package manager)

2. **Installation:**

   From the **`python/`** directory:

   ```bash
   # Create and activate a virtual environment (optional but recommended)
   python -m venv venv
   source venv/bin/activate   # On Windows: venv\Scripts\activate

   # Install the package and its dependencies (required to run the game)
   pip install -e .
   ```

   On Windows, you can also use the provided setup script:
   ```bash
   setup.bat
   ```

   Dependencies are defined in `pyproject.toml` (e.g. `readchar`, `tabulate`).

## Running the Game

The game supports two main commands: `play` for playing games and `benchmark` for comparing AI players. Run these from any directory **after** installing the package (see above).

### Play Command

Run individual games or multiple games with a specific player:

```bash
# Using the installed console script (recommended)
game2048 play [options]

# Or as a module (from any directory after pip install -e .)
python -m game2048.main play [options]
```

**Without installing the package** (e.g. for development): install runtime deps with `pip install readchar tabulate`, then from the `python/` directory:

```bash
PYTHONPATH=src python -m game2048.main play [options]
```

**Play command options:**
- `-n, --num_games`: Number of games to play (default: 1)
- `-p, --player`: Player type (random, maxempty, minmax, heuristic, human)
- `--profile_en`: Enable profiling
- `-v, --verbose`: Enable debug logging

### Benchmark Command

Compare the performance of different AI players:

```bash
game2048 benchmark [options]
# or: python -m game2048.main benchmark [options]
```

**Benchmark command options:**
- `-n, --num_games`: Number of games per player (default: 100)
- `--players`: Specific players to benchmark (if not specified, all non-human players are tested)
- `--optimize`: Enable board optimizations for faster execution
- `-o, --output`: Output file for benchmark results
- `--format`: Output format (text or html)
- `-v, --verbose`: Enable debug logging

The benchmark command generates detailed reports including:
- Performance metrics (average score, max score, moves per game, time per game)
- Highest tile distribution statistics
- Best game boards for each player
- Optional HTML report with interactive visualizations

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

5. **Benchmarking:**
   - Automated testing of multiple AI strategies
   - Performance metrics collection
   - Statistical analysis
   - HTML and text report generation

## Running Tests

From the **`python/`** directory, run tests with pytest (the project config sets `pythonpath = ["src"]`):

```bash
cd python
pytest tests/
```

For test coverage reports:
```bash
coverage run -m pytest tests/
coverage report
coverage html  # Generates HTML report in htmlcov/
```

## Customization

- **Creating Custom Players:**  
  Extend the `Player` class and implement the `get_move()` method to create your own AI strategy.

- **Visualization Customization:**  
  Modify the visualization settings in the interfaces module to change the appearance of the game.

- **Benchmark Reports:**
  The HTML reports can be customized by modifying the templates in `benchmark.py`.

---

Return to [Main README](../README.md)
