#!/bin/bash

#source Venv.source
source .venv/bin/activate
export PYTHONPATH=src
exec python3 -u src/main/bot.py
