#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-capture-report.sh

set -eu

if [ "$#" -ne 1 ]; then
	echo "usage: $0 CAPTURE"
	exit 1
fi

if [ ! -x ./build/kilonode-compat ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

./build/kilonode-compat capture-report "$1"
