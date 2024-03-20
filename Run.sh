#!/bin/bash

set -eu -o pipefail

#source Venv.source
source .venv/bin/activate
mypy gtb
export PYTHONPATH=.
#exec python3 -u gtb/inspect/print_html_wallet.py
#exec python3 -u gtb/inspect/print_html_pnl.py
#exec python3 -u gtb/inspect/print_html_orders.py
#exec python3 -u gtb/inspect/print_html_history.py
#exec python3 -u gtb/inspect/print_html_spread_stats.py
#exec python3 -u gtb/inspect/graph.py
exec python3 -u gtb/main.py
