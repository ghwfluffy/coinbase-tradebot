#!/bin/bash

set -eu -o pipefail

cat orderbook.json | jq .orders[].sell.market | cut -d'"' -f2 | sort -n
