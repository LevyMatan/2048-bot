# 2048 Bot

This project is a bot designed to play the 2048 game. It utilizes algorithms to make decisions based on the current state of the game board, aiming to achieve the highest possible score.

## Project Structure

```
2048-bot
├── src
│   ├── bot.py        # Implementation of the bot's decision-making logic
│   ├── game.py       # Manages game logic, including board and tile movements
│   ├── utils.py      # Utility functions for game assistance
│   └── types
│       └── index.py  # Custom types and data structures
├── requirements.txt   # Python dependencies for the project
└── README.md          # Documentation for the project
```

## Setup Instructions

1. Clone the repository:
   ```
   git clone <repository-url>
   cd 2048-bot
   ```

2. Install the required dependencies:
   ```
   pip install -r requirements.txt
   ```

## Usage Guidelines

To run the bot, execute the following command in the terminal:
```
python src/bot.py
```

## Bot Functionality

The bot analyzes the game board and makes moves based on a set of strategies to maximize the score. It evaluates possible moves and selects the most advantageous one, aiming to reach the 2048 tile and beyond.

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.