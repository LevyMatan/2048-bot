if __name__ == "__main__" and __package__ is None:
    from os import path
    import sys
    sys.path.insert(0, path.dirname(path.dirname(path.abspath(__file__))))
    __package__ = "src"

import random
import logging
from .players import Player, RandomPlayer, MaxEmptyCellsPlayer, HumanPlayer, HeuristicPlayer, MinMaxPlayer
from .board import Board
from .interfaces import GUI2048, CLI2048, GYM2048


MOVE_COUNT_PROGRESS_PRINT = 500

logger = logging.getLogger(__name__)

class Game2048:
    def __init__(self, board: Board | None = None, player: Player | None = None, move_count: int = 0, interface=None):
        if board is None:
            self.board = Board()
            # Only add initial random tiles if creating a new board
            while len(Board.get_empty_tiles(self.board.get_state())) > 14:
                self.add_random_tile()
        else:
            self.board = board

        self.move_count: int = move_count
        self.player = player if player else RandomPlayer()
        self.interface = interface

    def add_random_tile(self):
        current_state = self.board.get_state()
        empty_tiles = Board.get_empty_tiles(current_state)
        logger.debug("Empty tiles: %s", empty_tiles)
        if not empty_tiles:
            return
        row, col = random.choice(empty_tiles)
        logger.debug("Chosen tile: %s", (row, col))
        new_state = Board.set_tile(current_state, row, col, 1 if random.random() < 0.9 else 2)
        self.board.set_state(new_state)

    def play_move(self):
        valid_actions = Board.get_valid_move_actions(self.board.get_state())
        if valid_actions:
            action, next_state = self.player.choose_action(valid_actions)
            self.board.set_state(next_state)
            self.add_random_tile()
            return True
        return False

    def get_score(self) -> int:
        return sum([2 ** tile for tile in self.board.get_state(unpack=True) if tile > 0])

    def reset(self):
        self.board = Board()
        self.move_count = 0
        self.add_random_tile()
        self.add_random_tile()

    def play_game(self):
        self.reset()
        self.interface.display_initial_board(self.board.get_state())
        while self.play_move():
            self.move_count += 1
            if self.interface:
                self.interface.update(state=self.board.get_state(), move_count=self.move_count)
            # if self.move_count % MOVE_COUNT_PROGRESS_PRINT == 0:
            #     logger.debug(f"Move count: {self.move_count}")
        return self.get_score(), self.board.get_state(), self.move_count

if __name__ == "__main__":
    import cProfile
    import pstats
    import argparse

    # Define a mapping from string to Player subclass.
    player_mapping = {
        cls.__name__.replace("Player", "").lower(): cls
        for cls in [
            RandomPlayer,
            HumanPlayer,
            MaxEmptyCellsPlayer,
            MinMaxPlayer,
            HeuristicPlayer,
        ]
    }

    parser = argparse.ArgumentParser(description="2048 Game")
    parser.add_argument("--profile_en", action="store_true", help="Enable profiling")
    parser.add_argument("-n", "--num_games", type=int, default=1, help="Number of games to play")
    parser.add_argument("-p", "--player", type=str, default="human",
                        help=f"Player type {list(player_mapping.keys())}")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable debug logging")
    args = parser.parse_args()

    if args.profile_en:
        profiler = cProfile.Profile()
        profiler.enable()
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    # Choose the player from the mapping; use Human player as default
    player_key = args.player.lower()
    if player_key in player_mapping:
        player_cls = player_mapping[player_key]
    else:
        logger.info(f"Unknown player type '{args.player}'. Using Human player instead.")
        logger.info(f"Available player types: {list(player_mapping.keys())}")
        player_cls = HumanPlayer

    # If the player is Human, force the number of games to be 1
    if player_cls == HumanPlayer:
        args.num_games = 1

    if args.num_games > 1:
        interface = GYM2048()
        # Disable verifiers for improved performance when running multiple games
        Board.disable_verifiers()
    else:
        interface = CLI2048()

    game = Game2048(player=player_cls(), interface=interface)
    best_score = 0
    best_state = None
    best_move_count = 0
    num_of_games = args.num_games
    logger.info(f"Playing {num_of_games} games with {player_cls.__name__}...")

    for i in range(num_of_games):
        score, state, move_count = game.play_game()
        if score > best_score:
            best_score = score
            best_state = state
            best_move_count = move_count
        game.reset()

    if args.profile_en:
        profiler.disable()
        from datetime import datetime
        date_time = datetime.now().strftime("%Y%m%d_%H%M%S")
        profile_filename = f'{player_cls.__name__}_{args.num_games}_{date_time}.txt'
        with open(profile_filename, 'w') as f:
            stats = pstats.Stats(profiler, stream=f).sort_stats('cumulative')
            stats.print_stats()

    logger.info("Best game:")
    CLI2048.pretty_print(best_state, best_score, best_move_count)
