#!/bin/bash

set -e

source Venv.source
python3 ./mockresults.py ../data/mockresults.jsonl -o mockresults.png
