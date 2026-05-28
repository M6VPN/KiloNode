#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-safety-check.sh

set -eu

status=0
compat="./build/kilonode-compat"

require_file()
{
	if [ ! -f "$1" ]; then
		printf 'FAIL missing %s\n' "$1"
		status=1
	fi
}

check_output_has()
{
	name="$1"
	output="$2"
	needle="$3"

	case "$output" in
	*"${needle}"*) ;;
	*)
		printf 'FAIL %s missing %s\n' "$name" "$needle"
		status=1
		;;
	esac
}

require_file docs/safety/README.md
require_file docs/safety/ax25-response-safety-checklist.md
require_file docs/safety/ax25-prepared-to-tx-gate.md
require_file docs/safety/ax25-response-bench-gate.md
require_file docs/safety/ax25-operator-preflight.md
require_file docs/safety/ax25-no-transmit-regression.md
require_file docs/safety/ax25-live-scheduler-smoke-safety.md
require_file docs/safety/fx25-safety-placeholders.md
require_file tests/fixtures/safety/ax25-response-safety.required
require_file tests/fixtures/safety/ax25-response-safety.blocked
require_file tests/fixtures/safety/ax25-response-safety.report.expected

if ! ./scripts/ax25-no-transmit-check.sh; then
	status=1
fi

if ! ./scripts/ax25-live-scheduler-smoke-check.sh; then
	status=1
fi

if [ ! -x "$compat" ]; then
	printf 'ERR missing %s; run cmake --build build\n' "$compat"
	exit 1
fi

bench_output="$("$compat" replay-bench-prepared \
	tests/fixtures/bench/manifest.bench --expect \
	tests/fixtures/bench/prepared-frames.expected)"
check_output_has "bench-prepared" "$bench_output" "tx_writes=0"

timer_output="$("$compat" run-ax25-timer-prepared-dir \
	tests/fixtures/ax25-timer)"
check_output_has "timer-prepared" "$timer_output" "tx_writes=0"
check_output_has "timer-prepared" "$timer_output" "prepared_tx_writes=0"

diag_output="$("$compat" replay-bench-diagnostics \
	tests/fixtures/bench/manifest.bench)"
check_output_has "bench-diagnostics" "$diag_output" "tx_writes=0"

if [ "$status" -eq 0 ]; then
	printf 'OK ax25-safety-check docs=true fixtures=true replay=true tx_writes=0\n'
fi

exit "$status"
