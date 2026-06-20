#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/hobbyist-smoke.sh

set -eu

log=/tmp/kilonode-hobbyist-smoke.log
: > "$log"

run_quiet()
{
	name="$1"
	shift

	if "$@" >>"$log" 2>&1; then
		printf 'OK %s\n' "$name"
		return
	fi

	printf 'FAIL %s\n' "$name"
	printf 'Log: %s\n' "$log"
	exit 1
}

if [ ! -x ./build/kilonode-compat ] || [ ! -x ./build/kilonoded ]; then
	run_quiet build ./scripts/build.sh
fi

run_quiet config ./build/kilonoded --config packaging/examples/kilonode-hobbyist-local.conf --check-config
run_quiet loopback ./build/kilonode-compat run-ax25-loopback tests/fixtures/ax25-loopback/connect-i-rr-disconnect.loop
run_quiet connect-dry-run ./build/kilonode-compat run-ax25-connect-dry-run tests/fixtures/ax25-connect-dry-run/basic.connectdry
run_quiet bbs-local ./build/test_bbs_shell_store
run_quiet no-transmit ./scripts/ax25-no-transmit-check.sh

printf 'OK hobbyist-smoke hardware=false daemon_started=false tx_writes=0 dispatch=0 fx25=0\n'
