# 2048 Bot

A high-performance 2048 AI exploring multiple strategies — from simple heuristics to a **temporal-difference learning** agent with n-tuple networks that regularly reaches the **8192 tile** and beyond.

This project contains both C++ and Python implementations, plus a **browser-based GUI** for playing, training, and learning how the AI works.

| | |
|---|---|
| **[Documentation](docs/index.html)** | Technical deep-dive into the TDL player, n-tuple networks, and TD(0) learning |
| **[Play in Browser](docs/play.html)** | Interactive GUI with all AI players, Teacher Mode, and in-browser training |

## Project Structure

```
2048-bot/
├── cpp/                        # C++ implementation (primary, high-performance)
│   ├── src/
│   │   ├── main.cpp            # Entry point & game runner
│   │   ├── board.cpp/hpp       # 64-bit board representation
│   │   ├── game.cpp/hpp        # Game loop & mechanics
│   │   ├── random_player.cpp   # Random move selection
│   │   ├── heuristic_player.cpp# Weighted heuristic evaluation
│   │   ├── expectimax_player.cpp# Expectimax search
│   │   ├── tdl_player.cpp/hpp  # TD-Learning player + multi-threaded training
│   │   ├── ntuple_network.hpp  # N-Tuple network (4×6-tuple, 8-fold symmetry)
│   │   ├── evaluation.cpp/hpp  # Heuristic evaluation functions
│   │   └── arg_parser.cpp/hpp  # CLI argument parsing
│   ├── tests/                  # Google Test suite
│   └── CMakeLists.txt          # Build configuration
├── python/                     # Python implementation (prototyping)
├── docs/
│   ├── index.html              # Documentation site
│   └── play.html               # Browser GUI (play, train, learn)
└── README.md
```

## Player Types

| Player | Strategy | Avg Score | 8192 Rate |
|--------|----------|-----------|-----------|
| **Random** | Random legal move | ~1,000 | 0% |
| **Heuristic** | Weighted features (empty cells, monotonicity, smoothness, corner) | ~8,000 | 0% |
| **Expectimax** | Depth-limited search with chance nodes | ~20,000 | <1% |
| **TDL** | N-tuple network trained via TD(0) self-play | ~135,000 | ~70-75% |

## Quick Start

### Build

```bash
cd cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Play Games

```bash
# 10 games with heuristic player
./build/2048 -n 10 -p heuristic

# 100 games with TDL player (requires trained weights)
./build/2048 -n 100 -p tdl --weights weights.bin
```

### Train the TDL Network

```bash
# Train from scratch (100K games, saves to weights.bin)
./build/2048 --train --episodes 100000 --alpha 0.1 --weights weights.bin

# Continue training with lower learning rate
./build/2048 --train --episodes 100000 --alpha 0.01 --weights weights.bin

# Use all CPU cores (Hogwild parallel training)
./build/2048 --train --episodes 100000 --alpha 0.1 --weights weights.bin -t 10
```

The `--weights` flag is both load and save path — if the file exists, training **resumes** from it. Use `-t` to parallelize across cores (Apple M4: `-t 10`).

### Play in Browser

Open `docs/play.html` in any browser. Features:
- **Manual play** with keyboard/touch controls
- **AI players** (Random, Heuristic, Expectimax, TDL) with speed control
- **Load trained weights** (`weights.bin`) for the TDL player
- **Teacher Mode** — see exactly how each AI computes its next move
- **In-browser training** — train the network live and watch the TD(0) backward pass
- **Save/load weights** — export trained weights compatible with the C++ program

## How the TDL Player Works

The TDL player uses an **n-tuple network** — a pattern-based value function that maps board states to expected future scores.

### Evaluation (32 table lookups)

```
V(board) = Σ over 4 patterns × 8 symmetries of weights[index(board, pattern, symmetry)]
```

- **4 patterns**: fixed sets of 6 board positions (e.g., `[0,1,2,3,4,5]`)
- **8 symmetries**: rotations + mirrors (one shared weight table per pattern)
- **Each table**: 16⁶ = 16.7M entries (total: ~256 MB)

### Move Selection

```
best_move = argmax over moves of [ reward(move) + V(afterstate(move)) ]
```

### Training (TD(0) backward pass)

After each self-play game, walk **backward** through every move:

```
target = 0  (game over)
for each step from last to first:
    error = target - V(afterstate)
    V(afterstate) += alpha × error   (updates 32 weight entries)
    target = reward + V(afterstate)
```

This is how **preparation moves** (no immediate reward) learn to be valuable — the backward pass carries credit from future merges back to earlier states.

### Multi-Threaded Training

Training supports **Hogwild-style parallelism**: multiple threads play independent games and update the shared weight tables without locks. This works because individual updates are tiny (`alpha × error / 32`) and TD learning tolerates minor races. Scales nearly linearly with core count.

## Documentation

- **[Full Documentation](docs/index.html)** — project structure, board representation, all player types, CLI reference, and a deep dive into n-tuple networks, isomorphic symmetry, afterstate values, and TD(0) learning with worked examples.
- **[Browser GUI](docs/play.html)** — interactive play with Teacher Mode that visualizes the TDL evaluation pipeline (pattern lookups, symmetry mappings, shared tables) and the training backward pass.

## License

[MIT License](LICENSE)
