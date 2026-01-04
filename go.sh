#!/bin/bash

set -eux -o pipefail

LOGFILE="./data/log.txt"
if [ $# -gt 0 ]; then
    LOGFILE="${1}"
fi

make
rm -f "${LOGFILE}"

ARGS=(
    -v 2
    -m
    --log-file "${LOGFILE}"
    --debug-logs
    --trade-logs
)

#gdb --args \
./build/tradebot "${ARGS[@]}"
