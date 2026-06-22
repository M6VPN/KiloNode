#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/external-modem-check-configs.sh

set -eu

status=0
checked=0

configs="
packaging/examples/kilonode-external-modem-status.conf
packaging/examples/kilonode-mercury-planned.conf
packaging/examples/kilonode-hobbyist-local.conf
packaging/examples/kilonode-hobbyist-v0.2-alpha.conf
packaging/examples/kilonode-mercury-discovery.conf
packaging/examples/kilonode-rx-bench-tcp-kiss.conf
"

fail()
{
	printf 'FAIL %s\n' "$1"
	status=1
}

check_external_modem_block()
{
	file="$1"
	key="$2"
	bad="$3"

	if awk -v key="$key" -v bad="$bad" '
		$1 == "external-modem" && $3 == "{" { in_block = 1; next }
		in_block && $1 == "}" { in_block = 0; next }
		in_block && $1 == key && $2 == bad { found = 1 }
		END { exit found ? 0 : 1 }
	' "$file"; then
		fail "$file enables external-modem.${key} ${bad}"
	fi
}

if [ ! -x ./build/kilonoded ]; then
	printf 'FAIL missing ./build/kilonoded; run ./scripts/build.sh\n'
	exit 1
fi

for file in $configs; do
	if [ ! -f "$file" ]; then
		fail "missing config $file"
		continue
	fi
	check_external_modem_block "$file" tx-enabled true
	check_external_modem_block "$file" connect-enabled true
	check_external_modem_block "$file" auto-start true
	if ! ./build/kilonoded --config "$file" --check-config; then
		fail "$file check-config failed"
	fi
	checked=$((checked + 1))
done

if [ "$status" -eq 0 ]; then
	printf 'OK external-modem-configs checked=%s tx=false connect=false autostart=false launch=false\n' "$checked"
fi

exit "$status"
