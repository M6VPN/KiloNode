#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-kiss-tcp-smoke.sh

set -eu

config="packaging/examples/kilonode-rx-bench-tcp-kiss.conf"

if [ ! -x ./build/kilonoded ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

./build/kilonoded --config "$config" --check-config >/dev/null

echo "OK tcp-kiss-smoke config=$config"
echo "manual source: provide receive-only TCP KISS on 127.0.0.1:8001"
echo "manual monitor: ./scripts/bench-rx-monitor-tcp.sh 127.0.0.1 8001"
