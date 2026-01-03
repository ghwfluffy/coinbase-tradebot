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

# Warm load agent context
notice "Revising repository description"
runwithterm "${CODEX[@]}" "review the information in docs and describe what this code base does"

notice "Loading order pair and market architecture"
runwithterm "${CODEX[@]}" "summarize what an order pair is, and how the order pair market engine works"

notice "Revising market time details"
runwithterm "${CODEX[@]}" "summarize the intention of MarketConfFactory::rampedStockHours"

notice "Loading version2 trader summary"
runwithterm "${CODEX[@]}" "summarize what current traders are enabled for Version2 algorithm and what effect they will have"

set +e

CODEX_THOUGHT_FILE="docs/experiment/codex01.md"

# Instructions
BASE_PROMPT="$(cat <<CODEX
You are a senior quant/dev working inside this repository. Your job is to iteratively improve the bot’s profitability AND the stability of profit across runs.

Hard constraints:
- You SHOULD adjust what traders are instantiated in the Version2 configuration
- You MAY modify SpreadTrader, TimeTrader, VolumeTrader, and their configuration wiring including:
    - OrderPairMarketEngine
    - MarketInfo
    - MarketConfFactory
    - TraderConfFactory
- You SHOULD add new configurations to MarketConfFactory and TraderConfFactory and tweak their parameters.
- You MUST commit all changes and include a commit message with the iteration number

Loop protocol (do this every iteration):
1) Review ${CODEX_THOUGHT_FILE} and remember previous goals and attempts.
2) Parse LOG_FILE which contains new logs from the previous run.
    - the timestamps will overlap with previous runs because each iteration is being run against historical market data.
    - the file size can be several gigabytes and can not be read in full
    - use the techniques from docs/debug/logparse.md to parse the log file
    - refine and invent techniques and save the changes to logparse.md each iteration
    Extract:
    - total profit (and per-trader profit)
    - times of day/week when profits took a big loss
    - times of day/week when profits took a large win
    - drawdown / volatility proxy (if present)
    - number of trades, win rate, avg PnL, largest loss, fee/slippage effects
    - any warnings/errors or suspicious behavior (e.g., runaway trading, missing config)
3) Calculate how that time of day/week relates to tracked market times (MarketInfo::Market)
    - Determine if a new tracked market time should be added (rare, repeated occurrences required)
4) Create a new hypothesis on changes that can be made so the next iteration will be better. including changes to one or all of:
    - Add new Market type
    - Add new MarketTimeTraderConfig to MarketConfFactory
    - Modify existing MarketTimeTraderConfig in MarketConfFactory
    - Add new trader configuration in TraderConfFactory
    - Modify existing trader configuration in TraderConfFactory
    - Add/Modify/Remove traders instantiated in Version2 algorithm
5) Implement the smallest change that tests the hypothesis:
    - tweak existing params OR add a new config variant in MarketConfFactory/TraderConfFactory
    - adjust trader logic only if necessary, minimally, and explain why
6) Add debug logs ONLY where they increase decision quality next iteration:
    - log config id/name used, per-trader PnL, trade counts, reasons for entries/exits, spread/volume/time signals
    - update docs/debug/logparse.md with instructions on how logs are intended to be parsed and used
7) Update ${CODEX_THOUGHT_FILE} with:
    - iteration number + timestamp
    - extracted metrics (from log)
    - extracted observations (from log)
    - hypothesis
    - expected effect and what would falsify it
    - next step ideas
8) Git commit changes and ${CODEX_THOUGHT_FILE} with a descriptive commit message

Optimization target:
- Maximize risk-adjusted upward trend, not just final profit.
- Prefer changes that reduce variance and improve consistency.
- Do not get stuck on a single hypothesis for more than 3 iterations.
- The trading bot SHOULD run multiple traders with different strategries
- The trading bot SHOULD reach between \$1M and \$100M worth of volume per 30 day window
    - The 30-day volume is shown in the top STATUS line, and the total volume per trader is shown on the STATUS PnL lines
- The trading bot MAY experience negative profit initially to churn while the trading volume increases and the fee tier decreases.
CODEX
)"

plan() {
    local iter="${1}"
    local ts="${2}"
    local logfile="${3}"
    echo "${BASE_PROMPT}" | sed "s;LOG_FILE;${logfile};g"
    echo ""
    echo "You are in an iterative optimization loop. This is iteration ${iter} at ${ts}."
    echo "Follow the protocol strictly and keep changes minimal."
    if [ -f docs/experiment/new-input.md ]; then
        cat docs/experiment/new-input.md
    fi
}

#rm -rf data/runs
mkdir -p data/runs

# Iterate
iter=4
while true; do
    let "iter=iter+1" || true
    ts="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"

    notice "=== ITER ${iter} @ ${ts} ==="

    # Run bot
    LOGFILE="./data/log-${iter}.txt"
    if [ ${iter} -gt 5 ]; then
        runwithterm ./go.sh "${LOGFILE}" || true
    fi

    # Run the AI “agent” with the plan
    notice "Running codex"
    runwithterm "${CODEX[@]}" "$(plan "${iter}" "${ts}" "${LOGFILE}")" || true

    # Save log file for future analysis
    notice "Saving log file for ITER ${iter}"
    runwithterm zstd -T0 -10 -o "data/runs/${iter}.txt.zstd" "${LOGFILE}"
    rm -f "${LOGFILE}"

    runwithterm sleep 1
done
