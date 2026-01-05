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

CODEX_THOUGHT_FILE="docs/experiment/codex02.md"

# Instructions
BASE_PROMPT="$(cat <<CODEX
You are a senior quant/dev working inside this repository. Your job is to iteratively improve the bot’s profitability AND the stability of profit across runs. In this current iteration set we are looking to optimize the VolumeTrader.

Volume Trader goals:
- Trade very frequently in order to push 30-day volume above \$5M to unlock good maker fee tier
- Lose at most \$1K per month to reach that volume

Loop protocol (do this every iteration):
1) Review ${CODEX_THOUGHT_FILE} and remember previous goals and attempts.
2) Review docs/debug/quant.md to see findings of previous iterations
3) Parse LOG_FILE which contains new logs from the previous run.
    - use the techniques from docs/debug/logparse.md to parse the log file
    - refine and invent techniques and save the changes to logparse.md each iteration
4) Calculate how the time of day/week relates to tracked market times (MarketInfo::Market)
    - Determine if a new tracked market time should be added
5) Create a new hypothesis on changes that can be made so the next iteration will be better
6) Implement a change that tests the hypothesis
7) Add debug logs ONLY where they increase decision quality next iteration
    - update docs/debug/logparse.md with instructions on how logs are intended to be parsed and used
8) Update ${CODEX_THOUGHT_FILE} with:
    - iteration number + timestamp
    - extracted metrics (from log)
    - extracted observations (from log)
    - hypothesis
    - expected effect and what would falsify it
    - next step ideas
9) Git commit changes and ${CODEX_THOUGHT_FILE} with a descriptive commit message
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

mkdir -p data/runs/codex02

# Iterate
iter=0
while true; do
    let "iter=iter+1" || true
    ts="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"

    notice "=== ITER ${iter} @ ${ts} ==="

    # Run bot
    LOGFILE="./data/log-${iter}.txt"
    runwithterm ./go.sh "${LOGFILE}" || true

    # Run the AI “agent” with the plan
    notice "Running codex"
    runwithterm "${CODEX[@]}" "$(plan "${iter}" "${ts}" "${LOGFILE}")" || true

    # Save log file for future analysis
    notice "Saving log file for ITER ${iter}"
    runwithterm zstd -T0 -10 -o "data/runs/codex02/${iter}.txt.zstd" "${LOGFILE}"
    rm -f "${LOGFILE}"

    runwithterm sleep 1
done
