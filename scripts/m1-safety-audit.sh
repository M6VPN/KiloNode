#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/m1-safety-audit.sh

set -eu

status=0

run_check()
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

require_file()
{
	if [ ! -f "$1" ]; then
		printf 'FAIL missing %s\n' "$1"
		status=1
	fi
}

require_file docs/safety/ax25-response-safety-checklist.md
require_file docs/safety/ax25-no-transmit-regression.md
require_file docs/safety/ax25-live-scheduler-smoke-safety.md
require_file tests/fixtures/milestones/m1-blocked-features.txt

run_check ./scripts/ax25-no-transmit-check.sh
run_check ./scripts/ax25-safety-check.sh
run_check ./scripts/ax25-prepared-gate-report.sh
run_check ./scripts/ax25-live-scheduler-smoke-check.sh

if [ "$status" -eq 0 ]; then
	printf 'OK m1-safety-audit tx_writes=0 bridge_blocked=true connect=false dispatch=false fx25=false\n'
fi

exit "$status"
