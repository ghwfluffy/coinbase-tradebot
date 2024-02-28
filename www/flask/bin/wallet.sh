#!/bin/bash

set -eu -o pipefail

TOPDIR="$(readlink -f "$(dirname "$(readlink -f "${0}")")/../../../")"

cd "${TOPDIR}/coinbase"
source .venv/bin/activate
python3 src/print_wallet.py 2>&1
