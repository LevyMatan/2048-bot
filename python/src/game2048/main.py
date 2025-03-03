#!/usr/bin/env python3

import argparse
import cProfile
import logging
import pstats
from datetime import datetime
from collections import defaultdict

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

    # Benchmark command
    benchmark_parser = subparsers.add_parser("benchmark", help="Run benchmarks")
    benchmark_parser.add_argument("-n", "--num_games", type=int, default=100, help="Number of games per player")
    benchmark_parser.add_argument("--players", type=str, nargs="+", help="Specific players to benchmark")
    benchmark_parser.add_argument("--optimize", action="store_true", help="Enable board optimizations")
    benchmark_parser.add_argument("-o", "--output", type=str, help="Output file for benchmark results")
    benchmark_parser.add_argument("--format", type=str, choices=["text", "html"], default="text",
                               help="Output format (default: text)")
    benchmark_parser.add_argument("-v", "--verbose", action="store_true", help="Enable debug logging")

    args = parser.parse_args()

    # Set up logging
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    if args.command == "play":
        play_games(args)
    elif args.command == "benchmark":
        # Get available players
        available_players = []
        player_mapping = get_player_mapping()
        
        if args.players:
            # Use specified players
            for p in args.players:
                p_lower = p.lower()
                if p_lower in player_mapping:
                    available_players.append(player_mapping[p_lower])
                else:
                    logger.error(f"Unknown player: {p}")
                    logger.error(f"Available players: {list(player_mapping.keys())}")
                    return 1
        else:
            # Use all non-human players
            available_players = [cls for name, cls in player_mapping.items() if cls != HumanPlayer]
        
        if not available_players:
            logger.error("No valid players specified")
            return 1
            
        # Run benchmark with the selected players
        results = benchmark.run_benchmark(
            num_games=args.num_games,
            players=available_players,
            optimize=args.optimize,
            show_progress=True
        )
        
        # Generate report in requested format
        if args.format == "html":
            report = benchmark.generate_html_report(results)
        else:
            report = benchmark.generate_report(results)
        
        # Display report if not HTML
        if args.format == "text":
            print(report)
        
        # Save report to file if requested
        if args.output:
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(report)
            logger.info(f"Benchmark results saved to {args.output}")
            if args.format == "html":
                logger.info("Open the HTML file in a web browser to view the report")
    else:
        parser.print_help()
        return 1

    return 0

if __name__ == "__main__":
    raise SystemExit(main()) 