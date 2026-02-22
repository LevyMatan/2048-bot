# Benchmark metrics (8K repeatability)

## Success criteria
- **Pass:** at least 30% 8K hit rate over 500 games.

## Baseline (run 1)
- Config: `benchmark_sim_config.json` (500 games, 4 threads) + `benchmark_player_config.json` (Expectimax, depth 4, adaptive).
- Results: `baseline_results_500.json`
  - numGames: 500
  - hitRate4K: 0
  - hitRate8K: 0
  - avgScore: 2512.18
  - p95Score: 7900
  - timePerGameMs: 1486.35

## Leaderboard (config + metrics)
| Config profile        | hitRate8K | hitRate4K | avgScore | p95Score | timePerGameMs |
|----------------------|-----------|-----------|----------|----------|---------------|
| baseline (depth 4)   | 0         | 0         | 2512     | 7900     | 1486          |
| depth4_time75        | 0         | 0         | ~1582*   | -        | -             |
| depth5_time100       | 0         | 0         | ~1344*   | -        | -             |
| depth5_time50_chance4| 0         | 0         | ~1256*   | -        | -             |
* From sweep (50 games). Best avgScore in short sweep: depth4_time75.

| expectimax_tuned_weights (old weights) | 0   | 0.02 | 6883  | 54948 | 2676  |
* 100 games. Tuned eval weights (best_eval_weights + best preset) improve 4K rate and scores.

| expectimax_tuned_weights (post tune_heuristic) | 0.0125 | 0.175 | 21777 | 80792 | 9058  |
* 80 games. **New best:** eval weights from `tune_heuristic` (emptyTiles 324, monotonicity 393, mergeability 221, patternMatching 62, cornerValue 0). Depth 4, timeLimit 100 ms. Reaches 8K in ~1–2.5% of games.

## Stability (top config: expectimax_tuned_weights)
- Current winning config: `configurations/expectimax_tuned_weights.json` (post–tune_heuristic weights, depth 4, time 100 ms). Versioned copy: `expectimax_tuned_weights_v1.json`.

## Reproducibility and regression
- **Runner:** From repo root, `cd cpp && scripts/run_benchmark.sh [baseline|tuned|full]`. Uses BENCHMARK_GAMES (default 500) and BENCHMARK_THREADS (default 4).
- **Regression guard:** `cd cpp && scripts/check_8k_regression.sh [min_hit_rate_8k] [num_games] [config]`. Fails if hitRate8K is below threshold.
- **CI:** `.github/workflows/benchmark-8k-regression.yml` runs weekly and on dispatch.
  - Uses TDL player (`configurations/tdl_config.json`). Trains the n-tuple network (100K episodes) then benchmarks 100 games.
  - Threshold: 10% 8K hit rate. TDL achieves ~17% after 100K episodes, giving comfortable margin.
  - Environment variables: `TRAIN_EPISODES`, `TRAIN_THREADS`, `BENCHMARK_THREADS`.
  - The script auto-trains when `weights.bin` is missing, so CI is fully self-contained.
