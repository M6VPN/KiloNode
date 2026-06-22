#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/v02-alpha-readiness-check.sh

set -eu

status=0

run_check()
{
	name="$1"
	shift

	printf 'CHECK %s\n' "$name"
	if "$@"; then
		printf 'OK %s\n' "$name"
	else
		printf 'FAIL %s\n' "$name"
		status=1
	fi
}

run_check test-suite ./scripts/test.sh
run_check hobbyist-smoke ./scripts/hobbyist-smoke.sh
run_check hobbyist-first-run ./scripts/hobbyist-first-run.sh
run_check external-modem-configs ./scripts/external-modem-check-configs.sh
run_check mercury-discovery ./scripts/mercury-discovery-check.sh
run_check no-transmit ./scripts/ax25-no-transmit-check.sh
run_check m2-readiness ./scripts/m2-readiness-check.sh
run_check packaging ./scripts/check-packaging.sh

if [ "$status" -eq 0 ]; then
	printf 'OK v02-alpha-readiness checks=true hardware=false tx=false connect=false modem_launch=false\n'
fi

exit "$status"
