#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-live-scheduler-smoke-check.sh

set -eu

status=0

run_test()
{
	test_bin="$1"
	if [ ! -x "$test_bin" ]; then
		printf 'ERR missing %s; run cmake --build build\n' "$test_bin"
		status=1
		return
	fi
	"$test_bin"
}

run_test ./build/test_ax25_scheduler_smoke
run_test ./build/test_ax25_scheduler_smoke_diag
run_test ./build/test_daemon_ax25_scheduler

if [ -x ./scripts/ax25-no-transmit-check.sh ]; then
	if ! ./scripts/ax25-no-transmit-check.sh; then
		status=1
	fi
else
	printf 'ERR missing ./scripts/ax25-no-transmit-check.sh\n'
	status=1
fi

if [ "$status" -eq 0 ]; then
	printf 'OK ax25-live-scheduler-smoke tests=true tx_writes=0 dispatch_calls=0 hardware=false\n'
fi

exit "$status"
