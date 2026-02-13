#!/usr/bin/env bash
# Regression guard: run a small benchmark and fail if 8K hit rate is below threshold.
# Usage: cd cpp && scripts/check_8k_regression.sh [min_hit_rate_8k] [num_games]
# Example: scripts/check_8k_regression.sh 0.0 50   (ensure we do not regress below 0%)
#          scripts/check_8k_regression.sh 0.30 100   (require 30% 8K over 100 games for CI)
# Exit 0 if hitRate8K >= threshold, else 1.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
THRESHOLD="${1:-0.0}"
SAMPLE_GAMES="${2:-50}"
CONFIG="${3:-configurations/expectimax_tuned_weights.json}"
OUT="$CPP_DIR/regression_check_result.json"
cd "$CPP_DIR"

echo "Regression check: min hitRate8K=$THRESHOLD, sample=$SAMPLE_GAMES games, config=$CONFIG"
./build/2048 -n "$SAMPLE_GAMES" -t 4 --player-config "$CONFIG" --benchmark-output "$OUT" --output none 2>/dev/null

HIT8="$(grep -o '"hitRate8K":[^,]*' "$OUT" | cut -d: -f2)"
echo "hitRate8K=$HIT8 (threshold=$THRESHOLD)"

if awk -v h="$HIT8" -v t="$THRESHOLD" 'BEGIN { exit (h >= t) ? 0 : 1 }' 2>/dev/null; then
  echo "PASS: 8K rate meets or exceeds threshold."
  exit 0
fi
echo "FAIL: 8K rate below threshold."
exit 1
