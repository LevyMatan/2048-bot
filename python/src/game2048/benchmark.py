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
    RandomPlayer, MaxEmptyCellsPlayer, MinMaxPlayer, HeuristicPlayer, HumanPlayer
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
    
    # Set up GYM interface for progress tracking
    interface = GYM2048()
    total_games = len(players) * num_games
    interface.set_total_games(total_games)
    
    verifiers_disabled = False
    if optimize:
        Board.disable_verifiers()
        verifiers_disabled = True
    
    try:
        for player_cls in players:
            player_name = player_cls.__name__.replace("Player", "")
            logger.info(f"Benchmarking {player_name} for {num_games} games...")
            
            game = Game2048(player=player_cls(), interface=interface)
            scores = []
            highest_tiles = []
            move_counts = []
            best_score = 0
            best_state = None
            best_moves = 0
            
            # Track time for this player
            start_time = time.time()
            
            for _ in range(num_games):
                score, state, move_count = game.play_game()
                scores.append(score)
                highest_tiles.append(get_highest_tile(state))
                move_counts.append(move_count)
                
                if score > best_score:
                    best_score = score
                    best_state = state
                    best_moves = move_count
                game.reset()
            
            # Calculate time statistics
            total_time = time.time() - start_time
            time_per_game = total_time / num_games
            
            # Store results
            results[player_name] = {
                "avg_score": sum(scores) / num_games,
                "max_score": best_score,
                "best_state": best_state,
                "best_moves": best_moves,
                "avg_moves": sum(move_counts) / num_games,
                "highest_tile_counts": defaultdict(int),
                "time_per_game": time_per_game,
                "total_time": total_time
            }
            
            # Count tile frequencies
            for tile in highest_tiles:
                results[player_name]["highest_tile_counts"][tile] += 1
        
        # Add newline after progress updates
        if show_progress:
            print("\n")
            
        return results
    finally:
        # Re-enable verifiers if they were disabled
        if verifiers_disabled:
            Board.enable_verifiers()
            logger.debug("Board verifiers re-enabled after benchmarking")

def generate_report(results):
    """Generate a formatted report from benchmark results."""
    # Basic performance table
    performance_table = []
    headers = ["Player", "Games", "Avg Score", "Max Score", "Avg Moves", "Time/Game (s)"]
    
    for player, data in results.items():
        total_games = sum(data["highest_tile_counts"].values())
        performance_table.append([
            player,
            total_games,
            f"{data['avg_score']:.1f}",
            data['max_score'],
            f"{data['avg_moves']:.1f}",
            f"{data['time_per_game']:.3f}"
        ])
    
    # Highest tile distribution table
    tile_headers = ["Player"] + [str(2**i) for i in range(1, 12)]  # Just the tile values
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
    
    # Add best game visualizations
    report += "BEST GAMES:\n"
    for player, data in results.items():
        report += f"\n{player} (Score: {data['max_score']}, Moves: {data['best_moves']}):\n"
        import io
        import sys
        
        original_stdout = sys.stdout
        string_buffer = io.StringIO()
        sys.stdout = string_buffer
        
        CLI2048.pretty_print(data['best_state'], data['max_score'], data['best_moves'])
        
        board_string = string_buffer.getvalue()
        sys.stdout = original_stdout
        
        report += board_string + "\n"
    
    return report

def generate_html_board(state, score, moves):
    """Generate an HTML representation of a 2048 game board."""
    # 2048 game colors for each tile value
    tile_colors = {
        0: ('#cdc1b4', '#776e65'),     # Empty cell
        2: ('#eee4da', '#776e65'),     # 2
        4: ('#ede0c8', '#776e65'),     # 4
        8: ('#f2b179', '#f9f6f2'),     # 8
        16: ('#f59563', '#f9f6f2'),    # 16
        32: ('#f67c5f', '#f9f6f2'),    # 32
        64: ('#f65e3b', '#f9f6f2'),    # 64
        128: ('#edcf72', '#f9f6f2'),   # 128
        256: ('#edcc61', '#f9f6f2'),   # 256
        512: ('#edc850', '#f9f6f2'),   # 512
        1024: ('#edc53f', '#f9f6f2'),  # 1024
        2048: ('#edc22e', '#f9f6f2'),  # 2048
    }

    # Get the unpacked state
    unpacked_state = Board.get_unpacked_state(state)
    board = []
    for i in range(0, 16, 4):
        row = []
        for j in range(4):
            value = 2 ** unpacked_state[i + j] if unpacked_state[i + j] > 0 else 0
            bg_color, text_color = tile_colors.get(value, ('#3c3a32', '#f9f6f2'))  # Default for higher values
            row.append((value, bg_color, text_color))
        board.append(row)

    html = f"""
    <div class="game-container">
        <div class="game-info">
            <div class="score-box">Score: {score}</div>
            <div class="moves-box">Moves: {moves}</div>
        </div>
        <div class="grid-container">"""
    
    for row in board:
        html += '<div class="grid-row">'
        for value, bg_color, text_color in row:
            text = str(value) if value > 0 else ''
            html += f'''
            <div class="grid-cell" style="background-color: {bg_color}; color: {text_color}">
                {text}
            </div>'''
        html += '</div>'
    
    html += """
        </div>
    </div>"""
    
    return html

def generate_html_report(results):
    """Generate an HTML formatted report from benchmark results."""
    current_date = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    html = f"""<!DOCTYPE html>
<html>
<head>
    <title>2048 Benchmark Results</title>
    <style>
        body {{ 
            font-family: Arial, sans-serif; 
            margin: 20px; 
            background-color: #ffffff;
            color: #333333;
            line-height: 1.6;
        }}
        table {{ 
            border-collapse: collapse; 
            margin: 20px 0; 
            width: 100%; 
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }}
        th, td {{ 
            border: 1px solid #e0e0e0; 
            padding: 12px; 
            text-align: left; 
        }}
        th {{ 
            background-color: #f5f5f5; 
            font-weight: bold;
        }}
        tr:nth-child(even) {{
            background-color: #fafafa;
        }}
        .section {{ 
            margin: 40px 0;
            padding: 20px;
            background-color: #ffffff;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        h1, h2, h3 {{ 
            color: #333333;
            margin-top: 0;
        }}
        h1 {{
            border-bottom: 2px solid #bbada0;
            padding-bottom: 10px;
        }}
        .player-best {{
            background-color: #ffffff;
            padding: 20px;
            border-radius: 8px;
            margin: 15px 0;
            border: 1px solid #e0e0e0;
        }}
        .timestamp {{
            color: #666666;
            font-style: italic;
        }}
        .game-container {{
            width: 400px;
            margin: 20px auto;
        }}
        .game-info {{
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
        }}
        .score-box, .moves-box {{
            background: #bbada0;
            padding: 10px 20px;
            border-radius: 3px;
            color: white;
            font-weight: bold;
        }}
        .grid-container {{
            background: #bbada0;
            padding: 15px;
            border-radius: 6px;
            width: 100%;
            box-sizing: border-box;
        }}
        .grid-row {{
            display: flex;
            gap: 15px;
            margin-bottom: 15px;
        }}
        .grid-row:last-child {{
            margin-bottom: 0;
        }}
        .grid-cell {{
            width: 80px;
            height: 80px;
            border-radius: 3px;
            display: flex;
            justify-content: center;
            align-items: center;
            font-size: 24px;
            font-weight: bold;
            transition: background-color 0.15s ease;
        }}
    </style>
</head>
<body>
    <h1>2048 Benchmark Results</h1>
    <p class="timestamp">Generated on: {current_date}</p>
    
    <div class="section">
        <h2>Performance Metrics</h2>
        <table>
            <tr>
                <th>Player</th>
                <th>Games</th>
                <th>Avg Score</th>
                <th>Max Score</th>
                <th>Avg Moves</th>
                <th>Time/Game (s)</th>
            </tr>"""

    # Add performance data
    for player, data in results.items():
        total_games = sum(data["highest_tile_counts"].values())
        html += f"""
            <tr>
                <td>{player}</td>
                <td>{total_games}</td>
                <td>{data['avg_score']:.1f}</td>
                <td>{data['max_score']}</td>
                <td>{data['avg_moves']:.1f}</td>
                <td>{data['time_per_game']:.3f}</td>
            </tr>"""

    html += """
        </table>
    </div>
    
    <div class="section">
        <h2>Highest Tile Distribution</h2>
        <table>
            <tr>
                <th>Player</th>"""

    # Add simplified tile headers
    for i in range(1, 12):
        html += f"<th>{2**i}</th>"
    html += "</tr>"

    # Add tile distribution data
    for player, data in results.items():
        html += f"<tr><td>{player}</td>"
        counts = data["highest_tile_counts"]
        total_games = sum(counts.values())
        
        for i in range(1, 12):
            tile = 2**i
            count = counts.get(tile, 0)
            percentage = (count / total_games) * 100 if total_games > 0 else 0
            html += f"<td>{count} ({percentage:.1f}%)</td>"
        html += "</tr>"

    html += """
        </table>
    </div>
    
    <div class="section">
        <h2>Best Games</h2>"""

    # Add best games
    for player, data in results.items():
        html += f"""
        <div class="player-best">
            <h3>{player}</h3>"""
        
        # Generate HTML board directly
        html += generate_html_board(data['best_state'], data['max_score'], data['best_moves'])
        html += "</div>"

    html += """
    </div>
</body>
</html>
"""
    return html

def add_arguments(parser):
    """Add benchmark-specific arguments to the argument parser."""
    parser.add_argument("-n", "--num_games", type=int, default=100,
                      help="Number of games per player")
    parser.add_argument("--players", type=str, nargs="+",
                      help="Specific players to benchmark")
    parser.add_argument("--optimize", action="store_true",
                      help="Enable board optimizations")
    parser.add_argument("-o", "--output", type=str,
                      help="Output file for benchmark results")
    parser.add_argument("--format", type=str, choices=["text", "html"], default="text",
                      help="Output format (default: text)")
    parser.add_argument("-v", "--verbose", action="store_true",
                      help="Enable debug logging")

def handle_benchmark_command(args):
    """Handle the benchmark command from main.py."""
    # Configure logging
    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(level=log_level,
                      format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    try:
        # Get players to benchmark
        players_to_benchmark = get_available_players(args.players)
        
        logger.info(f"Starting benchmark with {len(players_to_benchmark)} players for {args.num_games} games each")
        
        # Run the benchmark
        results = run_benchmark(
            num_games=args.num_games,
            players=players_to_benchmark,
            optimize=args.optimize,
            show_progress=True
        )
        
        # Generate report in requested format
        if args.format == "html":
            report = generate_html_report(results)
        else:
            report = generate_report(results)
        
        # Display report if not HTML
        if args.format == "text":
            print(report)
        
        # Save report to file if requested
        if args.output:
            save_report(report, args.output, args.format)
            
    except ValueError as e:
        logger.error(str(e))
        return 1
        
    return 0

def main():
    """Main entry point when running benchmark.py directly."""
    parser = argparse.ArgumentParser(description="Benchmark 2048 AI players")
    add_arguments(parser)
    args = parser.parse_args()
    return handle_benchmark_command(args)

def get_available_players(specified_players=None, include_human=False):
    """
    Get a list of player classes based on specified names.
    
    Args:
        specified_players: List of player names to use, or None for all players
        include_human: Whether to include HumanPlayer in available players
        
    Returns:
        List of player classes to use for benchmarking
    
    Raises:
        ValueError: If no valid players are found or if an unknown player is specified
    """
    from .players import (
        RandomPlayer,
        HumanPlayer,
        MaxEmptyCellsPlayer,
        MinMaxPlayer,
        HeuristicPlayer,
    )
    
    # Define available players
    available_players = {
        cls.__name__.replace("Player", "").lower(): cls
        for cls in [
            RandomPlayer,
            MaxEmptyCellsPlayer,
            MinMaxPlayer,
            HeuristicPlayer,
        ]
    }
    
    if include_human:
        available_players["human"] = HumanPlayer
    
    if not specified_players:
        return list(available_players.values())
        
    # Use specified players
    selected_players = []
    for p in specified_players:
        p_lower = p.lower()
        if p_lower in available_players:
            selected_players.append(available_players[p_lower])
        else:
            raise ValueError(f"Unknown player: {p}. Available players: {list(available_players.keys())}")
    
    if not selected_players:
        raise ValueError(f"No valid players specified. Available players: {list(available_players.keys())}")
        
    return selected_players

def save_report(report, output_file, format="text"):
    """
    Save benchmark report to a file.
    
    Args:
        report: The report content (text or HTML)
        output_file: Path to save the report to
        format: Output format ("text" or "html")
    """
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(report)
    logger.info(f"Benchmark results saved to {output_file}")
    if format == "html":
        logger.info("Open the HTML file in a web browser to view the report")

if __name__ == "__main__":
    main()