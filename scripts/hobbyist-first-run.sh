#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/hobbyist-first-run.sh

set -eu

status=0

fail()
{
	printf 'FAIL %s\n' "$1"
	status=1
}

require_binary()
{
	if [ ! -x "$1" ]; then
		fail "missing $1; run ./scripts/build.sh"
	fi
}

require_script()
{
	if [ ! -x "$1" ]; then
		fail "missing executable script $1"
	fi
}

require_binary ./build/kilonoded
require_binary ./build/kilonodectl
require_binary ./build/kilonode-compat
require_script ./scripts/hobbyist-smoke.sh
require_script ./scripts/external-modem-check-configs.sh
require_script ./scripts/ax25-no-transmit-check.sh
require_script ./scripts/mercury-status-placeholder.sh

if [ "$status" -eq 0 ]; then
	./build/kilonoded --config packaging/examples/kilonode-hobbyist-v0.2-alpha.conf --check-config
	./scripts/hobbyist-smoke.sh
	./scripts/external-modem-check-configs.sh
	./scripts/ax25-no-transmit-check.sh
	./scripts/mercury-status-placeholder.sh
	printf 'INFO daemon status commands after starting local daemon:\n'
	printf 'INFO ./build/kilonodectl --socket /tmp/kilonode/control.sock status\n'
	printf 'INFO ./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates\n'
	printf 'INFO ./build/kilonodectl --socket /tmp/kilonode/control.sock modem-profiles\n'
	printf 'OK hobbyist-first-run config=true smoke=true modem_status=true tx=false connect=false hardware=false\n'
fi

exit "$status"
