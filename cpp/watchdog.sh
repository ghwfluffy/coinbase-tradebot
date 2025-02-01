#!/bin/bash
#
# Run the tradebot in a loop and create a massif memory profile
# when memory allocation starts running away
#
# memparse and allmemparse sources can be found in GhwFluffy's .ghw repository
#

set -eu -o pipefail

HIGH_HEAP_MEMORY=225000 # 225MB
HIGH_ALL_MEMORY=2529000 # 2.5GB

BOTPID=0
WATCHPID=0

cleanup() {
    kill -TERM "${WATCHPID}" 2>/dev/null || true
    kill -TERM "${BOTPID}" 2>/dev/null || true
    wait "${WATCHPID}" "${BOTPID}"
    exit 0
}
trap cleanup SIGINT SIGTERM

watchMemory() {
    while true; do
        sleep 1
        PID=($(ps -ef | grep tradebot | awk '{print $2}'))
        for pid in "${PID[@]}"; do
            if [ ! -f "/proc/${pid}/cmdline" ]; then
                continue
            fi
            # Find command being run
            CMD="$(cat "/proc/${pid}/cmdline" | sed 's;\x00;\n;g' | head -n1 || true)"
            if [ ! -z "${CMD}" ]; then
                CMD="$(basename "${CMD}")"
            fi
            # Only tradebot command
            if [ "${CMD}" != "valgrind.bin" ] && [ "${CMD}" != "tradebot" ]; then
                continue
            fi
            # Still alive?
            if ! kill -s 0 "${pid}" &> /dev/null; then
                continue
            fi

            # Check heap memory
            MEMORY="$(memparse "${pid}" | awk '{print $4}' || true)"
            if [ -n "${MEMORY}" ] && [ "${MEMORY}" -ge "${HIGH_HEAP_MEMORY}" ]; then
                kill -s USR2 "${pid}" || true
                continue
            fi

            # Check all memory (valgrind)
            MEMORY="$(allmemparse "${pid}" | awk '{print $3}' || true)"
            if [ -n "${MEMORY}" ] && [ "${MEMORY}" -ge "${HIGH_ALL_MEMORY}" ]; then
                kill -s USR2 "${pid}" || true
                continue
            fi
        done
    done
}

# Start memory watcher in the background
watchMemory &
WATCHPID=$!

# Run tradebot under massif
VALGRIND=(
    valgrind
    --tool=massif
    --detailed-freq=10
    --max-snapshots=1000
    --max-stackframe=50000000
)

while true; do
    "${VALGRIND[@]}" ./build/tradebot &
    BOTPID=$!
    wait "${BOTPID}"
done
