#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-connect-dry-run-fixtures.sh

set -eu

compat="./build/kilonode-compat"
fixture_dir="tests/fixtures/ax25-connect-dry-run"

if [ ! -x "$compat" ]; then
	printf 'FAIL missing %s; run cmake -S . -B build && cmake --build build\n' "$compat"
	exit 1
fi

for fixture in \
	"$fixture_dir/basic.connectdry" \
	"$fixture_dir/modulo-128.connectdry" \
	"$fixture_dir/paclen-window-boundary.connectdry"; do
	"$compat" check-ax25-connect-dry-run "$fixture"
	"$compat" run-ax25-connect-dry-run "$fixture"
done

if "$compat" check-ax25-connect-dry-run \
    "$fixture_dir/invalid-remote-callsign.connectdry" >/dev/null 2>&1; then
	printf 'FAIL invalid callsign fixture passed\n'
	exit 1
fi

if "$compat" run-ax25-connect-dry-run \
    "$fixture_dir/unsafe-tx-expectation.connectdry" >/dev/null 2>&1; then
	printf 'FAIL unsafe TX expectation fixture passed\n'
	exit 1
fi

printf 'OK ax25-connect-dry-run-fixtures hardware=false linbpq=false tx_writes=0 dispatch=0 fx25=0\n'
