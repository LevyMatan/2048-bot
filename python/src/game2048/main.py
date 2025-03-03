#!/usr/bin/env python3

import argparse
import cProfile
import logging
import pstats
from datetime import datetime

from .game import Game2048
from .board import Board
from .players import (
    Player,
    RandomPlayer,
    HumanPlayer,
    MaxEmptyCellsPlayer,
    MinMaxPlayer,
    HeuristicPlayer,
)
from .interfaces import CLI2048, GYM2048
from . import benchmark

logger = logging.getLogger(__name__)

def get_player_mapping() -> dict[str, type[Player]]:
    """Return a mapping of player name to player class."""
    return {
        cls.__name__.replace("Player", "").lower(): cls
        for cls in [
            RandomPlayer,
            HumanPlayer,
            MaxEmptyCellsPlayer,
            MinMaxPlayer,
            HeuristicPlayer,
        ]
    }

def play_games(args: argparse.Namespace) -> None:
    """Play the specified number of games with the chosen player."""
    if args.profile_en:
        profiler = cProfile.Profile()
        profiler.enable()

    player_mapping = get_player_mapping()
    player_key = args.player.lower()
    
    if player_key not in player_mapping:
        logger.error(f"Unknown player type '{args.player}'")
        logger.error(f"Available player types: {list(player_mapping.keys())}")
        raise SystemExit(1)
        
    player_cls = player_mapping[player_key]

    # If the player is Human, force the number of games to be 1
    if player_cls == HumanPlayer:
        args.num_games = 1

    if args.num_games > 1:
        interface = GYM2048()
        interface.set_total_games(args.num_games)
        Board.disable_verifiers()
    else:
        interface = CLI2048()

    game = Game2048(player=player_cls(), interface=interface)
    best_score = 0
    best_state = None
    best_move_count = 0
    
    logger.info(f"Playing {args.num_games} games with {player_cls.__name__}...")

    for _ in range(args.num_games):
        score, state, move_count = game.play_game()
        if score > best_score:
            best_score = score
            best_state = state
            best_move_count = move_count
        game.reset()

    # Add a newline after progress updates
    if args.num_games > 1:
        print("\n")

    if args.profile_en:
        profiler.disable()
        date_time = datetime.now().strftime("%Y%m%d_%H%M%S")
        profile_filename = f'{player_cls.__name__}_{args.num_games}_{date_time}.txt'
        with open(profile_filename, 'w') as f:
            stats = pstats.Stats(profiler, stream=f).sort_stats('cumulative')
            stats.print_stats()

    logger.info("Best game:")
    CLI2048.pretty_print(best_state, best_score, best_move_count, clear_screen=False)

def main() -> int:
    """Main entry point for the game."""
    parser = argparse.ArgumentParser(description="2048 Game")
    subparsers = parser.add_subparsers(dest="command", help="Command to run")

    # Play game command
    play_parser = subparsers.add_parser("play", help="Play 2048 game")
    play_parser.add_argument("--profile_en", action="store_true", help="Enable profiling")
    play_parser.add_argument("-n", "--num_games", type=int, default=1, help="Number of games to play")
    play_parser.add_argument("-p", "--player", type=str, default="human",
                          help=f"Player type {list(get_player_mapping().keys())}")
    play_parser.add_argument("-v", "--verbose", action="store_true", help="Enable debug logging")

    # Benchmark command - delegate to benchmark module
    benchmark_parser = subparsers.add_parser("benchmark", help="Run benchmarks")
    benchmark.add_arguments(benchmark_parser)

    args = parser.parse_args()

    # Set up logging
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    if args.command == "play":
        play_games(args)
    elif args.command == "benchmark":
        return benchmark.handle_benchmark_command(args)
    else:
        parser.print_help()
        return 1

    return 0

if __name__ == "__main__":
    raise SystemExit(main()) 