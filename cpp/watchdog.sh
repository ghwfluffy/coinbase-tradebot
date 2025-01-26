#!/bin/bash
#
# Run the tradebot in a loop and create a massif memory profile
# when memory allocation starts running away
#

HIGH_HEAP_MEMORY=125040
HIGH_ALL_MEMORY=2529000

watchMemory() {
    while true; do
        sleep 1
        PID=($(ps -ef | grep tradebot | grep -v 'grep\|cc1plus\|c++' | awk '{print $2}'))
        for pid in "${PID[@]}"; do
            MEMORY="$(memparse "${pid}" | awk '{print $4}')"
            if [ ! -z "${MEMORY}" ] && [ ${MEMORY} -ge ${HIGH_HEAP_MEMORY} ]; then
                kill -s USR2 "${pid}"
                sleep 20
                continue
            fi

            MEMORY="$(/home/tfuller/git/ghw/src/memparse/allmemparse "${pid}" | awk '{print $3}')"
            if [ ! -z "${MEMORY}" ] && [ ${MEMORY} -ge ${HIGH_ALL_MEMORY} ]; then
                kill -s USR2 "${pid}"
                sleep 20
                continue
            fi
        done
    done
}

watchMemory &

VALGRIND=(
    valgrind
    --tool=massif
    --detailed-freq=10
    --max-snapshots=1000
    --max-stackframe=50000000
)

while true; do
    "${VALGRIND[@]}" ./build/tradebot &
    PID=$!
    wait ${PID}
done
