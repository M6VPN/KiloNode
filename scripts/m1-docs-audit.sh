#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/m1-docs-audit.sh

set -eu

status=0

require_file()
{
	if [ ! -f "$1" ]; then
		printf 'FAIL missing %s\n' "$1"
		status=1
	fi
}

require_file docs/milestones/M1-v0.1-alpha-readiness.md
require_file docs/milestones/M1-feature-inventory.md
require_file docs/milestones/M1-test-inventory.md
require_file docs/milestones/M1-safety-audit.md
require_file docs/milestones/M1-compatibility-audit.md
require_file docs/milestones/M1-known-limitations.md
require_file docs/milestones/M1-next-milestones.md
require_file docs/safety/README.md
require_file docs/safety/ax25-response-safety-checklist.md
require_file docs/safety/ax25-no-transmit-regression.md
require_file docs/safety/ax25-live-scheduler-smoke-safety.md
require_file docs/bench/README.md
require_file docs/bench/diagnostic-replay.md
require_file docs/bench/ax25-live-scheduler-smoke.md
require_file docs/compat/ax25-conformance-matrix.md
require_file docs/compat/fx25-conformance-matrix.md
require_file docs/compat/linbpq-compatibility-matrix.md
require_file docs/compat/tnc-interface-matrix.md
require_file docs/compat/platform-matrix.md
require_file docs/man/kilonodectl.1
require_file docs/man/kilonode-compat.1
require_file tests/fixtures/milestones/m1-required-checks.txt
require_file tests/fixtures/milestones/m1-blocked-features.txt

if [ "$status" -eq 0 ]; then
	printf 'OK m1-docs-audit milestones=true safety=true bench=true compat=true man=true\n'
fi

exit "$status"
