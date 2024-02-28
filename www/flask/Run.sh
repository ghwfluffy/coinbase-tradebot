#!/bin/bash

set -eux -o pipefail

source .venv/bin/activate
.venv/bin/gunicorn -w 2 -b 127.0.0.1:8888 main:app
