#!/bin/bash

set -eux -o pipefail

make
rm -f ./data/log.txt

ARGS=(
    -v 2
    -m
    --log-file data/log.txt
    --debug-logs
    --trade-logs
)

#gdb --args \
./build/tradebot "${ARGS[@]}"
