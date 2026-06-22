#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/external-modem-status-smoke.sh

set -eu

socket="${1:-}"

if [ "$socket" = "" ]; then
	printf 'INFO provide a control socket path to query a running daemon\n'
	printf 'INFO example: ./scripts/external-modem-status-smoke.sh /tmp/kilonode/control.sock\n'
	exit 0
fi

if [ ! -x ./build/kilonodectl ]; then
	printf 'FAIL missing ./build/kilonodectl; run ./scripts/build.sh\n'
	exit 1
fi

./build/kilonodectl --socket "$socket" modem-profiles
./build/kilonodectl --socket "$socket" modems

printf 'OK external-modem-status-smoke socket=%s launch=false tx=false connect=false\n' "$socket"
