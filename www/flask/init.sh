#!/bin/bash

set -eux -o pipefail

python3 -m virtualenv .venv
source .venv/bin/activate
python3 -m pip install --upgrade pip
python3 -m pip install -r requirements.txt
