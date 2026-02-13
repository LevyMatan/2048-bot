#!/usr/bin/env bash
# After running tune_heuristic, merge best_eval_weights.json into Expectimax config.
# Usage: cd cpp && scripts/apply_tuned_weights.sh
# Reads configurations/best_eval_weights.json and updates configurations/expectimax_tuned_weights.json
# evalParams, preserving playerType, depth, timeLimit, etc.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BEST="$CPP_DIR/configurations/best_eval_weights.json"
OUT="$CPP_DIR/configurations/expectimax_tuned_weights.json"

if [ ! -f "$BEST" ]; then
  echo "Run tune_heuristic first to generate $BEST"
  exit 1
fi

# Build merged JSON: take existing expectimax_tuned_weights and replace evalParams from best_eval_weights.
# Use Python for reliable JSON merge if available.
if command -v python3 >/dev/null 2>&1; then
  python3 - "$BEST" "$OUT" << 'PY'
import json, sys
best_path, out_path = sys.argv[1], sys.argv[2]
with open(best_path) as f:
    best = json.load(f)
# Default Expectimax structure with evalParams from tuner
config = {
    "playerType": "Expectimax",
    "depth": 4,
    "chanceCovering": 2,
    "timeLimit": 75.0,
    "adaptiveDepth": True,
    "evalParams": {**{"monotonicity": 12, "smoothness": 29, "coreScore": -11, "patternMatching": 186}, **best}
}
with open(out_path, "w") as f:
    json.dump(config, f, indent=2)
print("Updated", out_path)
PY
else
  echo "python3 required to merge JSON. Copy $BEST eval keys into $OUT manually."
  exit 1
fi
