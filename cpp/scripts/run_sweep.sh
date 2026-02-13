#!/usr/bin/env bash
# Run parameter sweep: execute benchmark for each player config in sweep_configs/,
# then print leaderboard by hitRate8K, hitRate4K, avgScore.
# Usage: from repo root, run:  cd cpp && scripts/run_sweep.sh [num_games] [num_threads]
# Default: 100 games, 4 threads.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/sweep_configs"
RESULTS_DIR="$CPP_DIR/sweep_results"
NUM_GAMES="${1:-100}"
NUM_THREADS="${2:-4}"

mkdir -p "$RESULTS_DIR"
cd "$CPP_DIR"

echo "Sweep: $NUM_GAMES games, $NUM_THREADS threads"
echo "Configs: $CONFIGS_DIR"
echo "Results: $RESULTS_DIR"
echo ""

for f in "$CONFIGS_DIR"/*.json; do
  [ -f "$f" ] || continue
  name="$(basename "$f" .json)"
  out="$RESULTS_DIR/${name}.json"
  echo "Running $name ..."
  ./build/2048 -n "$NUM_GAMES" -t "$NUM_THREADS" --player-config "$f" --benchmark-output "$out" --output none 2>/dev/null || true
done

echo ""
echo "Leaderboard (by hitRate8K, then hitRate4K, then avgScore):"
echo "---"
for f in "$RESULTS_DIR"/*.json; do
  [ -f "$f" ] || continue
  name="$(basename "$f" .json)"
  hit8="$(grep -o '"hitRate8K":[^,]*' "$f" | cut -d: -f2)"
  hit4="$(grep -o '"hitRate4K":[^,]*' "$f" | cut -d: -f2)"
  avg="$(grep -o '"avgScore":[^,]*' "$f" | cut -d: -f2)"
  printf "%-30s hitRate8K=%s hitRate4K=%s avgScore=%s\n" "$name" "$hit8" "$hit4" "$avg"
done | sort -t= -k2 -rn -k4 -rn -k6 -rn
