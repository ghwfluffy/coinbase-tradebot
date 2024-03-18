#!/bin/bash

set -eu -o pipefail

TOPDIR="$(readlink -f "$(dirname "$(readlink -f "${0}")")/../../../")"

cd "${TOPDIR}"
source .venv/bin/activate
export PYTHONPATH=.
python3 -u gtb/inspect/print_html_history.py 2>&1
