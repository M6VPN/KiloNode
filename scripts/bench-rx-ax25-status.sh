#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-ax25-status.sh

set -eu

socket="${1:-/tmp/kilonode/control.sock}"

if [ "$#" -gt 1 ]; then
	echo "usage: $0 [control-socket]"
	exit 1
fi

if [ ! -x ./build/kilonodectl ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

if [ ! -S "$socket" ]; then
	echo "SKIP control socket not available: $socket"
	exit 0
fi

run_ctl()
{
	echo "$*"
	./build/kilonodectl --socket "$socket" "$@"
}

run_ctl status
run_ctl tx gates
run_ctl rx status
run_ctl heard
run_ctl ax25 live
run_ctl ax25 connections
