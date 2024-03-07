#!/bin/bash

set -eu -o pipefail

#source Venv.source
source .venv/bin/activate
export PYTHONPATH=src
(
    cd src
    mypy .
)
exec python3 -u src/main/bot.py
