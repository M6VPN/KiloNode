#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-timer-replay-fixtures.sh

set -eu

compat="./build/kilonode-compat"
fixtures="tests/fixtures/ax25-timer"

if [ ! -x "$compat" ]; then
	echo "error: missing $compat; run cmake -S . -B build && cmake --build build"
	exit 1
fi

"$compat" run-ax25-timer-replay-dir "$fixtures"
"$compat" run-ax25-timer-prepared-dir "$fixtures"
