#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/m2-readiness-check.sh

set -eu

status=0

run()
{
	if [ ! -x "$1" ]; then
		printf 'FAIL missing %s\n' "$1"
		status=1
		return
	fi
	if ! "$@"; then
		status=1
	fi
}

run ./scripts/ax25-loopback-fixtures.sh
run ./scripts/ax25-connect-dry-run-fixtures.sh
run ./scripts/ax25-no-transmit-check.sh

for test_bin in \
	./build/test_ax25_connect_dry_run \
	./build/test_ax25_connect_dry_run_script \
	./build/test_ax25_i_frame \
	./build/test_ax25_loopback_endpoint \
	./build/test_ax25_loopback_link \
	./build/test_ax25_loopback_payload \
	./build/test_ax25_loopback_retransmit \
	./build/test_ax25_loopback_script \
	./build/test_ax25_loopback_report \
	./build/test_ax25_loopback_segment \
	./build/test_ax25_loopback_window \
	./build/test_ax25_loopback \
	./build/test_ax25_paclen \
	./build/test_ax25_payload_delivery \
	./build/test_ax25_reassembly \
	./build/test_ax25_segmenter; do
	run "$test_bin"
done

if [ "$status" -eq 0 ]; then
	printf 'OK m2-readiness-check loopback=true window=true retransmit=true connect_dry_run=true hardware=false linbpq=false tx_writes=0\n'
fi

exit "$status"
