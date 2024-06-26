#!/bin/bash

set -eu -o pipefail

TOPDIR="$(readlink -f "$(dirname "$(readlink -f "${0}")")/../../../")"

cd "${TOPDIR}/coinbase"
source .venv/bin/activate
export PYTHONPATH=src:src/main
python3 src/inspect/print_allocations.py 2>&1
