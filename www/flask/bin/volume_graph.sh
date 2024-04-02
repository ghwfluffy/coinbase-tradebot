#!/bin/bash

set -eu -o pipefail

TOPDIR="$(readlink -f "$(dirname "$(readlink -f "${0}")")/../../../")"

cd "${TOPDIR}"
source .venv/bin/activate
export PYTHONPATH=.
python3 -u gtb/inspect/print_volume_graph.py 2>&1
