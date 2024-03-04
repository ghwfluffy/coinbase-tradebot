#!/bin/bash

set -eu -o pipefail

FLASK_DIR="$(readlink -f "$(dirname "$(readlink -f "${0}")")/../")"
COUNTER="${FLASK_DIR}/counter.txt"
if [ ! -f "${COUNTER}" ]; then
    COUNT=0
else
    COUNT="$(cat "${COUNTER}")"
fi

let "COUNT=COUNT+1" || true
echo "${COUNT}" > "${COUNTER}"

echo "${COUNT}"
