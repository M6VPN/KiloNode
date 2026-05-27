#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-fixture-status.sh

set -eu

manifest="${1:-tests/fixtures/bench/manifest.bench}"

if [ "$#" -gt 1 ]; then
	echo "usage: $0 [manifest]"
	exit 1
fi

if [ ! -x ./build/kilonode-compat ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

./build/kilonode-compat bench-coverage "$manifest"
