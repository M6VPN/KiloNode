#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/m1-readiness-check.sh

set -eu

status=0
compat="./build/kilonode-compat"

run_script()
{
	script="$1"
	if [ ! -x "$script" ]; then
		printf 'FAIL missing %s\n' "$script"
		status=1
		return
	fi
	if ! "$script"; then
		status=1
	fi
}

run_compat()
{
	if [ ! -x "$compat" ]; then
		printf 'FAIL missing %s; run cmake --build build\n' "$compat"
		status=1
		return
	fi
	if ! "$compat" "$@"; then
		status=1
	fi
}

run_script ./scripts/m1-docs-audit.sh
run_script ./scripts/m1-safety-audit.sh
run_script ./scripts/m1-compatibility-audit.sh
run_script ./scripts/bench-rx-replay-fixtures.sh
run_script ./scripts/bench-rx-replay-diagnostics.sh
run_script ./scripts/ax25-timer-replay-fixtures.sh
run_script ./scripts/ax25-prepared-replay-fixtures.sh
run_script ./scripts/ax25-live-scheduler-smoke-check.sh
run_script ./scripts/compat-validate-observation-pack.sh
run_script ./scripts/compat-check-node-plan.sh
printf 'INFO full-test-suite command=./scripts/test.sh separate=true\n'
run_compat replay-bench-prepared tests/fixtures/bench/manifest.bench \
	--expect tests/fixtures/bench/prepared-frames.expected
run_compat run-ax25-timer-prepared-dir tests/fixtures/ax25-timer

if [ "$status" -eq 0 ]; then
	printf 'OK m1-readiness-check result=ready hardware=false linbpq=false tx_writes=0\n'
fi

exit "$status"
