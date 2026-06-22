#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/hobbyist-status.sh

set -eu

socket="${1:-/tmp/kilonode/control.sock}"

if [ ! -x ./build/kilonodectl ]; then
	printf 'INFO missing ./build/kilonodectl; run ./scripts/build.sh\n'
	exit 0
fi

if ! ./build/kilonodectl --socket "$socket" status >/dev/null 2>&1; then
	printf 'INFO no daemon response on %s\n' "$socket"
	printf 'INFO validate config with: ./build/kilonoded --config packaging/examples/kilonode-hobbyist-v0.2-alpha.conf --check-config\n'
	printf 'INFO start daemon manually with a local non-transmitting config before querying status\n'
	exit 0
fi

./build/kilonodectl --socket "$socket" status
./build/kilonodectl --socket "$socket" ports
./build/kilonodectl --socket "$socket" tx gates
./build/kilonodectl --socket "$socket" ax25 status
./build/kilonodectl --socket "$socket" bbs status
./build/kilonodectl --socket "$socket" modem-profiles
./build/kilonodectl --socket "$socket" modems
printf 'OK hobbyist-status socket=%s\n' "$socket"
