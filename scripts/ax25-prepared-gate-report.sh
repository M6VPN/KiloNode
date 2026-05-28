#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-prepared-gate-report.sh

set -eu

compat="./build/kilonode-compat"
status=0

if [ -x "$compat" ]; then
	"$compat" check-prepared-expect \
		tests/fixtures/bench/prepared-frames.expected >/dev/null
else
	printf 'WARN missing %s; run cmake --build build for replay proof\n' \
		"$compat"
	status=1
fi

if grep -R -n -E 'prepared-bridge-to-tx[[:space:]]+true' \
	docs/examples packaging/examples; then
	printf 'FAIL prepared bridge enabled in example config\n'
	status=1
fi

if [ "$status" -eq 0 ]; then
	printf 'OK ax25-prepared-gate bridge=blocked real_tx=false fx25=false\n'
fi

exit "$status"
