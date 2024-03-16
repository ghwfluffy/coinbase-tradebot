#!/bin/bash

set -eu -o pipefail

#source Venv.source
source .venv/bin/activate
mypy gtb
export PYTHONPATH=.
exec python3 -u gtb/main.py
