#!/usr/bin/env bash
# Reproducible benchmark runner: single entrypoint for full experiment batch.
# Usage:
#   cd cpp && scripts/run_benchmark.sh [preset]
# Presets: baseline | tuned | full
#   baseline: 500 games, benchmark_player_config.json, writes baseline_results_500.json
#   tuned:    500 games, expectimax_tuned_weights.json, writes benchmark_tuned_500.json
#   full:     baseline + tuned (default)
# Optional env: BENCHMARK_GAMES=500, BENCHMARK_THREADS=4

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
GAMES="${BENCHMARK_GAMES:-500}"
THREADS="${BENCHMARK_THREADS:-4}"
PRESET="${1:-full}"
cd "$CPP_DIR"

run_one() {
  local name="$1"
  local config="$2"
  local out="$3"
  echo "Benchmark: $name ($GAMES games, $THREADS threads)"
  ./build/2048 -n "$GAMES" -t "$THREADS" --player-config "$config" --benchmark-output "$out" --output none 2>&1
  echo "Results: $out"
}

case "$PRESET" in
  baseline)
    run_one "baseline" "configurations/benchmark_player_config.json" "baseline_results_500.json"
    ;;
  tuned)
    run_one "tuned" "configurations/expectimax_tuned_weights.json" "benchmark_tuned_500.json"
    ;;
  full)
    run_one "baseline" "configurations/benchmark_player_config.json" "baseline_results_500.json"
    run_one "tuned" "configurations/expectimax_tuned_weights.json" "benchmark_tuned_500.json"
    ;;
  *)
    echo "Unknown preset: $PRESET. Use baseline | tuned | full"
    exit 1
    ;;
esac
