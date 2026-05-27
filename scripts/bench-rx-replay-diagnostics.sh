#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-replay-diagnostics.sh

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

expected_dir="$(dirname "$manifest")"
expected="${expected_dir}/ax25-diag-replay.expected"
prepared_expected="${expected_dir}/prepared-frames.expected"

if [ -f "$expected" ]; then
	./build/kilonode-compat check-bench-expected "$expected"
fi

./build/kilonode-compat replay-bench-diagnostics "$manifest"

if [ -f "$prepared_expected" ]; then
	./build/kilonode-compat check-prepared-expect "$prepared_expected"
	./build/kilonode-compat replay-bench-prepared "$manifest" --expect "$prepared_expected"
fi
