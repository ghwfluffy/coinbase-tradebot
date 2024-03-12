#!/bin/bash

source Venv.source
cd src
mypy --check-untyped-defs .
