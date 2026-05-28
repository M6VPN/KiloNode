#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-loopback-fixtures.sh

set -eu

compat="./build/kilonode-compat"
fixtures="tests/fixtures/ax25-loopback"

if [ ! -x "$compat" ]; then
	printf 'FAIL missing %s; run cmake --build build\n' "$compat"
	exit 1
fi

"$compat" run-ax25-loopback-dir "$fixtures"
printf 'OK ax25-loopback-fixtures hardware=false linbpq=false tx_writes=0 dispatch=0 fx25=0\n'
