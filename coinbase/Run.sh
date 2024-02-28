#!/bin/bash

#source Venv.source
source .venv/bin/activate
export PYTHONPATH=src
#python3 src/newmain.py
#python3 src/main.py
exec python3 -u src/3main.py
