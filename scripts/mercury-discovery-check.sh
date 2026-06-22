#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/mercury-discovery-check.sh

set -eu

status=0

fail()
{
	printf 'FAIL %s\n' "$1"
	status=1
}

require_file()
{
	if [ ! -f "$1" ]; then
		fail "missing $1"
	fi
}

require_file docs/modems/mercury-ofdm.md
require_file docs/modems/mercury-ofdm-discovery-pack.md
require_file docs/modems/mercury-ofdm-interface-questions.md
require_file docs/modems/mercury-ofdm-test-plan.md
require_file docs/modems/mercury-ofdm-capture-notes.md
require_file packaging/examples/kilonode-mercury-discovery.conf

if [ ! -x ./build/kilonoded ]; then
	fail "missing ./build/kilonoded; run ./scripts/build.sh"
else
	if ! ./build/kilonoded --config packaging/examples/kilonode-mercury-discovery.conf --check-config >/dev/null; then
		fail "mercury discovery config failed validation"
	fi
fi

if ! grep -R -q 'modem-profile mercury-ofdm' README.md docs/product docs/getting-started-hobbyist.md; then
	fail "mercury modem-profile command is not documented"
fi

if grep -n -E 'tx-enabled[[:space:]]+true|connect-enabled[[:space:]]+true|auto-start[[:space:]]+true' \
    packaging/examples/kilonode-mercury-discovery.conf; then
	fail "mercury discovery config enables unsafe modem action"
fi

if ! grep -q 'planned' docs/modems/mercury-ofdm-discovery-pack.md; then
	fail "mercury discovery pack does not mark status planned"
fi

if ! grep -q 'not implemented' docs/modems/mercury-ofdm-discovery-pack.md; then
	fail "mercury discovery pack does not mark implementation absent"
fi

if [ "$status" -eq 0 ]; then
	printf 'OK mercury-discovery docs=true config=true tx=false connect=false autostart=false launch=false\n'
fi

exit "$status"
