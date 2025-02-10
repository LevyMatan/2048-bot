# 2048 Bot

A bot to play the 2048 game using various AI strategies. The project explores different decision-making algorithms to achieve high scores, with some algorithms capable of reaching scores of 20,000 and beyond.

## Features

- **Multiple AI Strategies:**  
  Choose from several player types including:
  - `RandomPlayer`: Makes random moves.
  - `MaxEmptyCellsPlayer`: Selects moves that maximize empty cells.
  - `MinMaxPlayer`: Uses a simple min/max heuristic based on board values.
  - `HeuristicPlayer`: Applies a multi-factor heuristic considering empty cells, monotonicity, smoothness, and max tiles.
  - `MonteCarloPlayer`: Simulates random playouts to choose the best statistically averaged move.
  - `ExpectimaxPlayer`: Uses expectimax search with adjustable depth and weighted heuristics.

- **Customizable Search Depth & Weights:**  
  The `ExpectimaxPlayer` prints its configuration (depth and heuristic weights) at startup for transparency.

- **Modular Design:**  
  The project uses clearly separated modules for the game logic (`game.py`), board management (`board.py`), and player strategies (`players.py`). This allows easy tweaking and experimentation.

- **Profiling & Batch Play:**  
  Run multiple games and enable Python profiling to analyze performance.

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

1. **Clone the repository:**

   ```bash
   git clone <repository-url>
   cd 2048-bot
   ```

2. **Install dependencies:**

   ```bash
   pip install -r requirements.txt
   ```

## Running the Bot

Run the game using the following command. You can choose the player type and number of games via command-line arguments:

```bash
python src/game.py -n 1000 -p ExpectimaxPlayer
```

- **Arguments:**
  - `-n` or `--num_games`: Number of games to play (default is 1000).
  - `-p` or `--player`: The player algorithm to use. Available options are:
    - `randomplayer`
    - `humanplayer`
    - `maxemptycellsplayer`
    - `minmaxplayer`
    - `heuristicplayer`
    - `montecarloplayer`
    - `expectimaxplayer`
  - `--profile_en`: Enable Python profiling.

## How It Works

1. **Game Initialization:**  
   The `Game2048` class sets up the board and starts adding random tiles until a meaningful starting position is reached.

2. **AI Decision Making:**  
   The chosen `Player` subclass evaluates possible moves via its `choose_action` method. For example:
   - *ExpectimaxPlayer* recursively simulates moves (player and chance nodes) to forecast the best move based on a weighted heuristic.

3. **Board and Tile Management:**  
   The `Board` class handles all board operations including:
   - Simulating moves using precomputed lookup tables for efficiency.
   - Adding random tiles after each move.
   - Converting, printing, and evaluating board states.

4. **Game Play:**  
   The game loop in `game.py` repeatedly:
   - Gets valid moves.
   - Lets the AI decide the next move.
   - Updates the board and adds a random tile.
   - Tracks the move count and score.

5. **Output:**  
   At the end of the games, statistics such as the best score, move count, and game board are printed in a nicely formatted output.

## Customization & Contribution

- **Tweak AI Strategies:**  
  You can adjust parameters (lookahead depth, heuristic weights, simulation count) in the different player classes found in `players.py` to experiment with performance.
  
- **Board Improvements:**  
  The board logic in `board.py` is optimized using lookup tables. Feel free to extend this as needed.

- **Enhancements:**  
  Contributions to improve strategies, add new player types, or refine the scoring system are welcome. Fork the repository and submit a pull request with your updates.

## License

[MIT License](LICENSE) (if applicable)

---

Happy coding, and good luck reaching those 2048 and beyond! 🚀
