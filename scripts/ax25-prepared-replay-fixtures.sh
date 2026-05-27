#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-prepared-replay-fixtures.sh

set -eu

compat="./build/kilonode-compat"
bench_manifest="tests/fixtures/bench/manifest.bench"
bench_expect="tests/fixtures/bench/prepared-frames.expected"
timer_dir="tests/fixtures/ax25-timer"

if [ ! -x "${compat}" ]; then
	printf '%s\n' "ERR missing ${compat}; run cmake --build build"
	exit 1
fi

"${compat}" check-prepared-expect "${bench_expect}"
"${compat}" replay-bench-prepared "${bench_manifest}" --expect "${bench_expect}"
"${compat}" run-ax25-timer-prepared-dir "${timer_dir}"
