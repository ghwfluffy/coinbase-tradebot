#!/bin/bash

PYTHON=$(which python3.12 || which python3.10)

cd cex
source Venv.source
${PYTHON} test.py
