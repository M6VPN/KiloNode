#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-no-transmit-check.sh

set -eu

status=0

non_lab_configs="
docs/examples/kilonode.conf
packaging/examples/kilonode.conf
packaging/examples/kilonode-minimal.conf
packaging/examples/kilonode-bbs-local.conf
packaging/examples/kilonode-hobbyist-local.conf
packaging/examples/kilonode-hobbyist-v0.2-alpha.conf
packaging/examples/kilonode-external-modem-status.conf
packaging/examples/kilonode-mercury-planned.conf
packaging/examples/kilonode-mercury-discovery.conf
packaging/examples/kilonode-monitor-only.conf
packaging/examples/kilonode-rx-bench-tcp-kiss.conf
packaging/examples/kilonode-rx-bench-serial-kiss.conf
packaging/examples/kilonode-rx-bench-pty-kiss.conf
packaging/examples/kilonode-rx-bench-unix-kiss.conf
"

all_configs="
$non_lab_configs
packaging/examples/kilonode-tx-test-only.conf
packaging/examples/kilonode-tx-lab-only.conf
"

fail()
{
	printf 'FAIL %s\n' "$1"
	status=1
}

check_not_enabled_in_block()
{
	file="$1"
	block="$2"
	key="$3"
	bad="$4"
	value="$(awk -v block="$block" -v key="$key" -v bad="$bad" '
		$1 == block && $2 == "{" { in_block = 1; next }
		$1 == block && $3 == "{" { in_block = 1; next }
		in_block && $1 == "}" { in_block = 0; next }
		in_block && $1 == key && $2 == bad { found = 1 }
		END { print found ? "bad" : "ok" }
	' "$file")"

	if [ "$value" = "bad" ]; then
		fail "$file enables ${block}.${key} ${bad}"
	fi
}

for file in $non_lab_configs; do
	if [ ! -f "$file" ]; then
		fail "missing config $file"
		continue
	fi
	check_not_enabled_in_block "$file" transmit enabled true
	check_not_enabled_in_block "$file" transmit dispatch-real-kiss true
	check_not_enabled_in_block "$file" transmit dispatch-enabled true
	check_not_enabled_in_block "$file" ax25 prepared-bridge-to-tx true
	check_not_enabled_in_block "$file" ax25 live-scheduler-tx-actions true
	check_not_enabled_in_block "$file" ax25 live-scheduler-smoke-create-test-connection true
	check_not_enabled_in_block "$file" external-modem tx-enabled true
	check_not_enabled_in_block "$file" external-modem connect-enabled true
	check_not_enabled_in_block "$file" external-modem auto-start true
done

for file in $all_configs; do
	if [ ! -f "$file" ]; then
		fail "missing config $file"
		continue
	fi
	if grep -n -E 'prepared-bridge-enabled[[:space:]]+true' "$file" |
	    grep -q -E 'test-only[[:space:]]+false'; then
		fail "$file enables prepared bridge real mode"
	fi
	if grep -n -E 'connect-command[[:space:]]+true|live-connect[[:space:]]+true' \
	    "$file"; then
		fail "$file enables live connect"
	fi
	if grep -n -E 'live-scheduler-smoke[[:space:]]+true' "$file" |
	    grep -q -E 'prepared-bridge-to-tx[[:space:]]+true|live-scheduler-tx-actions[[:space:]]+true'; then
		fail "$file combines scheduler smoke with transmit actions"
	fi
done

if [ "$status" -eq 0 ]; then
	printf 'OK ax25-no-transmit configs=true bridge_disabled=true dispatch_default=false fx25=false\n'
fi

exit "$status"
