#!/bin/bash

set -eu -o pipefail

# Allow cancel with ctrl+C
PID=0
cleanup() {
    if [ ${PID} -gt 0 ]; then
        kill "${PID}"
        wait "${PID}" &> /dev/null
        echo ""
    fi
    exit 1
}
trap cleanup INT
trap cleanup TERM

runwithterm() {
    local ARGS=("${@}")
    "${ARGS[@]}" &
    PID=$!
    wait "${PID}"
    PID=0
}

notice() {
    echo -e "\e[1;5;32m${@}\e[0m"
}

# Start new codex session
notice "Loading repository description"
OUTPUT="$(codex exec --json "give a description of this repository")"
SESSION_ID="$(echo "${OUTPUT}" | head -n1 | jq -r 'select(.type=="thread.started") | .thread_id')"

# Codec command
CODEX=(
    codex
    exec
    resume
    "${SESSION_ID}"
)

# Make agent load some context
notice "Revising repository description"
runwithterm "${CODEX[@]}" "review the information in docs and describe what this code base does"

notice "Loading order pair and market architecture"
runwithterm "${CODEX[@]}" "summarize what an order pair is, and how the order pair market engine works"

notice "Loading version2 trader summary"
runwithterm "${CODEX[@]}" "summarize what current traders are enabled for Version2 algorithm and what effect they will have"

set +e

# Instructions
BASE_PROMPT="$(cat <<CODEX
You are a senior quant/dev working inside this repository. Your job is to iteratively improve the bot’s profitability AND the stability of profit across runs.

Hard constraints:
- Only modify: SpreadTrader, TimeTrader, VolumeTrader, and their configuration wiring.
- You MAY add/adjust MarketTimeTraderConfigs.
- You MAY add new configurations to MarketConfFactory and TraderConfFactory and tweak their parameters.
- No other strategy modules. No unrelated refactors. Keep diffs small and reversible.
- Never “optimize” by overfitting to a single lucky run. The profit must trend upward across iterations.

Loop protocol (do this every iteration):
1) Read data/log.txt from the previous run. Extract:
   - total profit (and any per-market/per-trader profit if available)
   - drawdown / volatility proxy (if present)
   - number of trades, win rate, avg PnL, largest loss, fee/slippage effects
   - any warnings/errors or suspicious behavior (e.g., runaway trading, missing config)
2) Read in data/runs/iter*.txt as necessary to check logs from previous iterations
3) Decide ONE primary hypothesis to test next (single variable change if possible).
4) Implement the smallest change that tests the hypothesis:
   - tweak existing params OR add a new config variant in MarketConfFactory/TraderConfFactory
   - adjust trader logic only if necessary, minimally, and explain why
5) Add debug logs ONLY where they increase decision quality next iteration:
   - log config id/name used, per-trader PnL, trade counts, reasons for entries/exits, spread/volume/time signals
6) Update docs/experiment.md with:
   - iteration number + timestamp
   - extracted metrics (from log)
   - hypothesis
   - change summary (files + key params)
   - expected effect and what would falsify it
   - next step ideas

Optimization target:
- Maximize risk-adjusted upward trend, not just final profit.
- Prefer changes that reduce variance and improve consistency.
- If results regress for 2 consecutive iterations, roll back the last change and try a different hypothesis.

Output format each iteration:
- “Observed” (metrics from data/log.txt)
- “Hypothesis”
- “Change” (exact edits planned)
- “Why this should help”
- “New debug logs”
- “Experiment.md entry” (append-ready)
- “Session ID” (${SESSION_ID})
CODEX
)"

plan() {
    local iter="$1"
    local ts="$2"
    if [ ${iter} -eq 1 ]; then
        echo "${BASE_PROMPT}"
    fi
    echo "You are in an iterative optimization loop. This is iteration ${iter} at ${ts}."
    echo "Follow the protocol strictly and keep changes minimal."
}

rm -rf data/runs
mkdir -p data/runs

# Iterate
iter=0
while true; do
    let "iter=iter+1" || true
    ts="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"

    notice "=== ITER ${iter} @ ${ts} ==="

    # Run bot
    runwithterm ./go.sh || true

    # Snapshot new log for trend analysis
    if [[ -f data/log.txt ]]; then
        cp -f data/log.txt "data/runs/iter${iter}.txt" || true
    fi

    # Run the AI “agent” with the plan
    runwithterm "${CODEX[@]}" "$(plan "${iter}" "${ts}")" || true

    runwithterm sleep 1
done
