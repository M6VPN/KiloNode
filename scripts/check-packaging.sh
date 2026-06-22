#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/check-packaging.sh

set -eu

status=0

require_file()
{
	if [ ! -f "$1" ]; then
		echo "missing packaging file: $1"
		status=1
	fi
}

require_section()
{
	file="$1"
	section="$2"

	if ! grep -q "^\\.Sh ${section}$" "$file"; then
		echo "missing man section ${section}: $file"
		status=1
	fi
}

check_config()
{
	file="$1"

	require_file "$file"
	if [ -x ./build/kilonoded ]; then
		if ! ./build/kilonoded --config "$file" --check-config >/dev/null; then
			echo "config failed validation: $file"
			status=1
		fi
	fi

	if grep -n -E 'host[[:space:]]+0\.0\.0\.0|host[[:space:]]+::' "$file"; then
		echo "public bind address found in example: $file"
		status=1
	fi
}

check_rx_bench_config()
{
	file="$1"

	check_config "$file"

	transmit_enabled="$(awk '
		$1 == "transmit" && $2 == "{" { in_tx = 1; next }
		in_tx && $1 == "}" { in_tx = 0; next }
		in_tx && $1 == "enabled" { print $2 }
	' "$file")"
	if [ "$transmit_enabled" != "false" ]; then
		echo "receive bench config enables transmit: $file"
		status=1
	fi

	if grep -n -E 'tx-enabled[[:space:]]+true|dispatch-enabled[[:space:]]+true' \
	    "$file"; then
		echo "receive bench config enables a TX gate: $file"
		status=1
	fi
}

check_manpage()
{
	file="$1"

	require_file "$file"
	for section in NAME SYNOPSIS DESCRIPTION OPTIONS FILES EXAMPLES \
		"SECURITY NOTES" "SEE ALSO" AUTHORS LICENSE; do
		require_section "$file" "$section"
	done
}

require_file packaging/systemd/kilonoded.service
require_file packaging/openbsd/kilonoded.rc
require_file packaging/freebsd/kilonoded
require_file packaging/netbsd/kilonoded

check_config packaging/examples/kilonode.conf
check_config packaging/examples/kilonode-minimal.conf
check_config packaging/examples/kilonode-bbs-local.conf
check_config packaging/examples/kilonode-hobbyist-local.conf
check_config packaging/examples/kilonode-external-modem-status.conf
check_config packaging/examples/kilonode-mercury-planned.conf
check_config packaging/examples/kilonode-monitor-only.conf
check_config packaging/examples/kilonode-tx-test-only.conf
check_config packaging/examples/kilonode-tx-lab-only.conf
check_rx_bench_config packaging/examples/kilonode-rx-bench-tcp-kiss.conf
check_rx_bench_config packaging/examples/kilonode-rx-bench-serial-kiss.conf
check_rx_bench_config packaging/examples/kilonode-rx-bench-pty-kiss.conf
check_rx_bench_config packaging/examples/kilonode-rx-bench-unix-kiss.conf

require_file docs/bench/README.md
require_file docs/bench/receive-only-safety.md
require_file docs/bench/direwolf-receive-only.md
require_file docs/bench/direwolf-usb-soundcard-kiss.md
require_file docs/bench/kilotnc-receive-only.md
require_file docs/bench/serial-kiss-receive-only.md
require_file docs/bench/tcp-kiss-receive-only.md
require_file docs/bench/pty-kiss-receive-only.md
require_file docs/bench/unix-socket-kiss-receive-only.md
require_file docs/bench/ax25-live-diagnostics-checklist.md
require_file docs/bench/fx25-future-bench-notes.md
require_file docs/bench/capture-fixtures.md
require_file docs/bench/importing-manual-captures.md
require_file docs/bench/replay-bench-captures.md
require_file docs/bench/diagnostic-replay.md
require_file docs/bench/manual-capture-workspace.md
require_file docs/bench/manual-capture-index.md
require_file docs/bench/manual-capture-replay.md
require_file docs/bench/fx25-capture-placeholders.md
require_file docs/bench/ax25-response-bench-gate.md
require_file docs/bench/ax25-prepared-response-validation.md
require_file docs/bench/ax25-future-tx-lab-notes.md
require_file docs/bench/vara-external-modem.md
require_file docs/bench/mercury-ofdm-external-modem.md
require_file docs/bench/ardop-external-modem.md
require_file docs/getting-started-hobbyist.md
require_file docs/modems/README.md
require_file docs/modems/direwolf-kiss.md
require_file docs/modems/kilotnc-kiss.md
require_file docs/modems/mercury-ofdm.md
require_file docs/modems/vara-hf-fm.md
require_file docs/modems/ardop.md
require_file docs/modems/external-modem-discovery-checklist.md

require_file docs/milestones/M1-v0.1-alpha-readiness.md
require_file docs/milestones/M1-feature-inventory.md
require_file docs/milestones/M1-test-inventory.md
require_file docs/milestones/M1-safety-audit.md
require_file docs/milestones/M1-compatibility-audit.md
require_file docs/milestones/M1-known-limitations.md
require_file docs/milestones/M1-next-milestones.md
require_file docs/milestones/M2.7-hobbyist-preview.md
require_file docs/milestones/M2.8-external-modem-roadmap.md
require_file docs/milestones/M2.8-external-modem-scaffold.md
require_file docs/release/v0.2-alpha-checklist.md

require_file docs/safety/README.md
require_file docs/safety/ax25-response-safety-checklist.md
require_file docs/safety/ax25-prepared-to-tx-gate.md
require_file docs/safety/ax25-response-bench-gate.md
require_file docs/safety/ax25-operator-preflight.md
require_file docs/safety/ax25-no-transmit-regression.md
require_file docs/safety/fx25-safety-placeholders.md

require_file scripts/bench-rx-replay-diagnostics.sh
require_file scripts/bench-rx-replay-fixtures.sh
require_file scripts/bench-rx-import-capture.sh
require_file scripts/bench-rx-capture-report.sh
require_file scripts/bench-rx-fixture-status.sh
require_file scripts/bench-rx-workspace-init.sh
require_file scripts/bench-rx-workspace-import.sh
require_file scripts/bench-rx-workspace-list.sh
require_file scripts/bench-rx-workspace-replay.sh
require_file scripts/bench-rx-workspace-report.sh
require_file scripts/ax25-safety-check.sh
require_file scripts/ax25-no-transmit-check.sh
require_file scripts/ax25-prepared-gate-report.sh
require_file scripts/ax25-response-bench-preflight.sh
require_file scripts/external-modem-check-configs.sh
require_file scripts/external-modem-status-smoke.sh
require_file scripts/hobbyist-smoke.sh
require_file scripts/m1-docs-audit.sh
require_file scripts/m1-test-inventory.sh
require_file scripts/m1-safety-audit.sh
require_file scripts/m1-compatibility-audit.sh
require_file scripts/m1-readiness-check.sh

require_file tests/fixtures/bench/README.md
require_file tests/fixtures/bench/manifest.bench
require_file tests/fixtures/bench/ax25-diag-replay.expected
require_file tests/fixtures/bench/kiss-ui-cq.capture
require_file tests/fixtures/bench/kiss-ui-ping-node.capture
require_file tests/fixtures/bench/kiss-sabm-node.capture
require_file tests/fixtures/bench/kiss-disc-node.capture
require_file tests/fixtures/bench/kiss-rr-node.capture
require_file tests/fixtures/bench/ax25-ui-cq.capture
require_file tests/fixtures/bench/ax25-sabm-node.capture
require_file tests/fixtures/bench/fx25-future-placeholder.capture
require_file tests/fixtures/bench/kiss-sabm-disc-sequence.capture
require_file tests/fixtures/bench/kiss-sabm-rr-disc-sequence.capture
require_file tests/fixtures/manual-captures/README.md
require_file tests/fixtures/manual-captures/workspace.manifest
require_file tests/fixtures/manual-captures/import-source/kiss-manual-ui.capture
require_file tests/fixtures/manual-captures/import-source/kiss-manual-sabm.capture
require_file tests/fixtures/manual-captures/import-source/fx25-manual-placeholder.capture
require_file tests/fixtures/safety/README.md
require_file tests/fixtures/safety/ax25-response-safety.required
require_file tests/fixtures/safety/ax25-response-safety.blocked
require_file tests/fixtures/safety/ax25-response-safety.report.expected
require_file tests/fixtures/milestones/README.md
require_file tests/fixtures/milestones/m1-required-checks.txt
require_file tests/fixtures/milestones/m1-blocked-features.txt

check_manpage docs/man/kilonoded.8
check_manpage docs/man/kilonodectl.1
check_manpage docs/man/kilonode-compat.1
check_manpage docs/man/kilonode-monitor.1
check_manpage docs/man/kilonode-msg.1
check_manpage docs/man/kilonode-user.1
check_manpage docs/man/kilonode-store.1

blocked_word="s""udo"
if grep -R -n "$blocked_word" packaging scripts/install-local.sh \
	scripts/uninstall-local.sh scripts/bench-rx-check-configs.sh \
	scripts/bench-rx-direwolf-notes.sh scripts/bench-rx-kiss-tcp-smoke.sh \
	scripts/bench-rx-monitor-tcp.sh scripts/bench-rx-ax25-status.sh \
	scripts/bench-rx-replay-diagnostics.sh \
	scripts/bench-rx-replay-fixtures.sh scripts/bench-rx-import-capture.sh \
	scripts/bench-rx-capture-report.sh scripts/bench-rx-fixture-status.sh \
	scripts/bench-rx-workspace-init.sh scripts/bench-rx-workspace-import.sh \
	scripts/bench-rx-workspace-list.sh scripts/bench-rx-workspace-replay.sh \
	scripts/bench-rx-workspace-report.sh scripts/ax25-safety-check.sh \
	scripts/ax25-no-transmit-check.sh scripts/ax25-prepared-gate-report.sh \
	scripts/ax25-response-bench-preflight.sh scripts/hobbyist-smoke.sh \
	scripts/external-modem-check-configs.sh \
	scripts/external-modem-status-smoke.sh \
	scripts/m1-docs-audit.sh scripts/m1-test-inventory.sh scripts/m1-safety-audit.sh \
	scripts/m1-compatibility-audit.sh scripts/m1-readiness-check.sh; then
	echo "packaging files must not contain privileged helper commands"
	status=1
fi

if ! ./scripts/ax25-no-transmit-check.sh; then
	status=1
fi

exit "$status"
