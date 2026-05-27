#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-check-configs.sh

set -eu

configs="
packaging/examples/kilonode-rx-bench-tcp-kiss.conf
packaging/examples/kilonode-rx-bench-serial-kiss.conf
packaging/examples/kilonode-rx-bench-pty-kiss.conf
packaging/examples/kilonode-rx-bench-unix-kiss.conf
"

if [ ! -x ./build/kilonoded ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

status=0

check_transmit_block()
{
	file="$1"
	value="$(awk '
		$1 == "transmit" && $2 == "{" { in_tx = 1; next }
		in_tx && $1 == "}" { in_tx = 0; next }
		in_tx && $1 == "enabled" { print $2 }
	' "$file")"

	if [ "$value" != "false" ]; then
		echo "FAIL transmit enabled in $file"
		status=1
	fi
}

for file in $configs; do
	if [ ! -f "$file" ]; then
		echo "FAIL missing $file"
		status=1
		continue
	fi

	if ! ./build/kilonoded --config "$file" --check-config >/dev/null; then
		echo "FAIL check-config $file"
		status=1
		continue
	fi

	check_transmit_block "$file"

	if grep -n -E 'host[[:space:]]+0\.0\.0\.0|host[[:space:]]+::' "$file"; then
		echo "FAIL public bind address in $file"
		status=1
	fi

	if grep -n -E 'tx-enabled[[:space:]]+true|dispatch-enabled[[:space:]]+true' \
	    "$file"; then
		echo "FAIL transmit gate enabled in $file"
		status=1
	fi

	echo "OK $file"
done

if [ "$status" -eq 0 ]; then
	echo "OK bench-rx-configs count=4 transmit=false"
fi

exit "$status"
