#!/bin/bash

set -eu -o pipefail

#source Venv.source
source .venv/bin/activate
export PYTHONPATH=src
(
    cd src
    mypy .
)
if [ ! -f "historical.json" ]; then
    echo '[]' > historical.json
fi
exec python3 -u src/main/bot.py
