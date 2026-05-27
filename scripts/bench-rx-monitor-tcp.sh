#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-monitor-tcp.sh

set -eu

host="${1:-127.0.0.1}"
port="${2:-8001}"

if [ "$#" -gt 2 ]; then
	echo "usage: $0 [host] [port]"
	exit 1
fi

if [ ! -x ./build/kilonode-monitor ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

echo "./build/kilonode-monitor --tcp-connect $host $port --max-frame 2048"
echo "This helper prints the receive-only monitor command and does not open the socket."
