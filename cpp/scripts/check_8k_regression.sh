#!/usr/bin/env bash
# Regression guard: run a benchmark and fail if 8K hit rate is below threshold.
#
# Usage: cd cpp && scripts/check_8k_regression.sh [min_hit_rate_8k] [num_games] [config]
#
# By default uses the TDL player.  When config points to a TDL player JSON
# whose weightsPath does not yet exist, the script trains the network first.
#
# Examples:
#   scripts/check_8k_regression.sh 0.30 100                        # TDL default
#   scripts/check_8k_regression.sh 0.01 30 configurations/expectimax_tuned_weights_v1.json
#
# Environment variables:
#   TRAIN_EPISODES   — training episodes (default: 100000)
#   TRAIN_THREADS    — threads for training  (default: 4)
#   BENCHMARK_THREADS — threads for benchmark (default: 4)
#
# Exit 0 if hitRate8K >= threshold, else 1.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
THRESHOLD="${1:-0.10}"
SAMPLE_GAMES="${2:-100}"
CONFIG="${3:-configurations/tdl_config.json}"
OUT="$CPP_DIR/regression_check_result.json"

TRAIN_EPISODES="${TRAIN_EPISODES:-100000}"
TRAIN_THREADS="${TRAIN_THREADS:-4}"
BENCHMARK_THREADS="${BENCHMARK_THREADS:-4}"

cd "$CPP_DIR"

# If the config is for a TDL player, train first when weights are missing.
if grep -q '"playerType".*"TDL"' "$CONFIG" 2>/dev/null; then
  WEIGHTS_PATH=$(grep -o '"weightsPath"[[:space:]]*:[[:space:]]*"[^"]*"' "$CONFIG" | \
                 sed 's/.*"weightsPath"[[:space:]]*:[[:space:]]*"\([^"]*\)"/\1/')
  WEIGHTS_PATH="${WEIGHTS_PATH:-weights.bin}"

  if [ ! -f "$WEIGHTS_PATH" ]; then
    echo "Training TDL network: $TRAIN_EPISODES episodes, $TRAIN_THREADS threads -> $WEIGHTS_PATH"
    ./build/2048 --train --episodes "$TRAIN_EPISODES" --weights "$WEIGHTS_PATH" \
                 --threads "$TRAIN_THREADS" --output none 2>/dev/null
    echo "Training complete."
  else
    echo "Using existing weights: $WEIGHTS_PATH"
  fi
fi

echo "Regression check: min hitRate8K=$THRESHOLD, sample=$SAMPLE_GAMES games, config=$CONFIG"
./build/2048 -n "$SAMPLE_GAMES" -t "$BENCHMARK_THREADS" --player-config "$CONFIG" \
             --benchmark-output "$OUT" --output none 2>/dev/null

HIT8="$(grep -o '"hitRate8K":[^,]*' "$OUT" | cut -d: -f2)"
echo "hitRate8K=$HIT8 (threshold=$THRESHOLD)"

if awk -v h="$HIT8" -v t="$THRESHOLD" 'BEGIN { exit (h >= t) ? 0 : 1 }' 2>/dev/null; then
  echo "PASS: 8K rate meets or exceeds threshold."
  exit 0
fi
echo "FAIL: 8K rate below threshold."
exit 1
