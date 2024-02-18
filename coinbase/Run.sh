#!/bin/bash

PYTHON=$(which python3.12 || which python3.10)

source Venv.source
export PYTHONPATH=src
"${PYTHON}" test.py
