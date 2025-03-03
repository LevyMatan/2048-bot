#!/usr/bin/env python3
"""
Benchmark script for comparing different 2048 AI players.
Runs each player for a specified number of games and reports performance metrics.
"""

import argparse
import logging
import time
from collections import defaultdict
from datetime import datetime
from tabulate import tabulate  # You may need to install this: pip install tabulate

from .game import Game2048
from .board import Board
from .players import (
    RandomPlayer, MaxEmptyCellsPlayer, MinMaxPlayer, HeuristicPlayer
)
from .interfaces import GYM2048, CLI2048

logger = logging.getLogger(__name__)

def get_highest_tile(state):
    """Returns the value of the highest tile on the board."""
    unpacked_state = Board.get_unpacked_state(state)
    if not unpacked_state:
        return 0
    highest_exponent = max(unpacked_state)
    return 2 ** highest_exponent if highest_exponent > 0 else 0

def run_benchmark(num_games, players, optimize=False, show_progress=True):
    """
    Run benchmark with specified players for the given number of games.
    
    Args:
        num_games: Number of games to run per player
        players: List of player classes to benchmark
        optimize: Whether to enable board optimization
        show_progress: Whether to show progress during benchmarking
        
    Returns:
        Dictionary with benchmark results
    """
    results = {}
    
    verifiers_disabled = False
    if optimize:
        Board.disable_verifiers()
        verifiers_disabled = True
    
    try:
        for player_cls in players:
            player_name = player_cls.__name__.replace("Player", "")
            logger.info(f"Benchmarking {player_name} for {num_games} games...")
            
            # Initialize metrics
            start_time = time.time()
            scores = []
            highest_tiles = []
            move_counts = []
            best_score = 0
            best_state = None
            best_moves = 0
            
            # Create game instance with silent interface
            game = Game2048(player=player_cls(), interface=GYM2048())
            
            # Run games
            for i in range(num_games):
                if show_progress and i > 0 and i % 10 == 0:
                    elapsed = time.time() - start_time
                    eta = (elapsed / i) * (num_games - i)
                    logger.info(f"{player_name}: {i}/{num_games} games completed. ETA: {eta:.1f}s")
                
                score, state, move_count = game.play_game()
                scores.append(score)
                highest_tiles.append(get_highest_tile(state))
                move_counts.append(move_count)
                
                if score > best_score:
                    best_score = score
                    best_state = state
                    best_moves = move_count
            
            elapsed_time = time.time() - start_time
            
            # Store results
            results[player_name] = {
                "avg_score": sum(scores) / num_games,
                "max_score": best_score,
                "best_state": best_state,
                "best_moves": best_moves,
                "avg_moves": sum(move_counts) / num_games,
                "highest_tile_counts": defaultdict(int),
                "time_per_game": elapsed_time / num_games,
                "total_time": elapsed_time
            }
            
            # Count tile frequencies
            for tile in highest_tiles:
                results[player_name]["highest_tile_counts"][tile] += 1
        
        return results
    finally:
        # Re-enable verifiers if they were disabled, ensuring cleanup even if exceptions occur
        if verifiers_disabled:
            Board.enable_verifiers()
            logger.debug("Board verifiers re-enabled after benchmarking")

def generate_report(results):
    """Generate a formatted report from benchmark results."""
    # Basic performance table
    performance_table = []
    headers = ["Player", "Avg Score", "Max Score", "Avg Moves", "Time/Game (s)"]
    
    for player, data in results.items():
        performance_table.append([
            player,
            f"{data['avg_score']:.1f}",
            data['max_score'],
            f"{data['avg_moves']:.1f}",
            f"{data['time_per_game']:.3f}"
        ])
    
    # Highest tile distribution table
    tile_headers = ["Player"] + [str(2**i) for i in range(1, 12)]  # Tiles from 2 to 2048
    tile_table = []
    
    for player, data in results.items():
        row = [player]
        counts = data["highest_tile_counts"]
        total_games = sum(counts.values())
        
        for i in range(1, 12):
            tile = 2**i
            count = counts.get(tile, 0)
            percentage = (count / total_games) * 100 if total_games > 0 else 0
            row.append(f"{count} ({percentage:.1f}%)")
        
        tile_table.append(row)
    
    # Generate the report
    report = "\n\n"
    report += "=" * 80 + "\n"
    report += f"BENCHMARK RESULTS ({datetime.now().strftime('%Y-%m-%d %H:%M:%S')})\n"
    report += "=" * 80 + "\n\n"
    
    report += "PERFORMANCE METRICS:\n"
    report += tabulate(performance_table, headers=headers, tablefmt="grid") + "\n\n"
    
    report += "HIGHEST TILE DISTRIBUTION:\n"
    report += tabulate(tile_table, headers=tile_headers, tablefmt="grid") + "\n\n"
    
    # Add best game visualizations - use pretty_print to generate string representation
    report += "BEST GAMES:\n"
    for player, data in results.items():
        report += f"\n{player} (Score: {data['max_score']}, Moves: {data['best_moves']}):\n"
        # Fix: Use a string buffer to capture the output of pretty_print
        import io
        import sys
        
        # Temporarily redirect stdout to capture pretty_print output
        original_stdout = sys.stdout
        string_buffer = io.StringIO()
        sys.stdout = string_buffer
        
        # Call the CLI2048.pretty_print method that exists
        CLI2048.pretty_print(data['best_state'], data['max_score'], data['best_moves'])
        
        # Get the captured output and restore stdout
        board_string = string_buffer.getvalue()
        sys.stdout = original_stdout
        
        report += board_string + "\n"
    
    return report

def main():
    parser = argparse.ArgumentParser(description="Benchmark 2048 AI players")
    parser.add_argument("-n", "--num_games", type=int, default=100, 
                        help="Number of games to run per player")
    parser.add_argument("--players", type=str, nargs="+", 
                        help="Specific players to benchmark (default: all)")
    parser.add_argument("--optimize", action="store_true", 
                        help="Enable board optimizations")
    parser.add_argument("-o", "--output", type=str, 
                        help="Output file for benchmark results")
    parser.add_argument("-v", "--verbose", action="store_true", 
                        help="Enable verbose logging")
    args = parser.parse_args()

    # Configure logging
    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(level=log_level, 
                        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # Define available players
    available_players = {
        "random": RandomPlayer,
        "maxemptycells": MaxEmptyCellsPlayer,
        "minmax": MinMaxPlayer,
        "heuristic": HeuristicPlayer
    }

    # Select players to benchmark
    if args.players:
        players_to_benchmark = []
        for p in args.players:
            p_lower = p.lower()
            if p_lower in available_players:
                players_to_benchmark.append(available_players[p_lower])
            else:
                logger.warning(f"Unknown player: {p}")
        
        if not players_to_benchmark:
            logger.error("No valid players specified. Available players: " + 
                         ", ".join(available_players.keys()))
            return
    else:
        players_to_benchmark = list(available_players.values())

    logger.info(f"Starting benchmark with {len(players_to_benchmark)} players for {args.num_games} games each")
    
    # Run the benchmark
    results = run_benchmark(
        num_games=args.num_games,
        players=players_to_benchmark,
        optimize=args.optimize,
        show_progress=True
    )
    
    # Generate and display the report
    report = generate_report(results)
    print(report)
    
    # Save report to file if requested
    if args.output:
        with open(args.output, 'w') as f:
            f.write(report)
        logger.info(f"Benchmark results saved to {args.output}")

if __name__ == "__main__":
    main()