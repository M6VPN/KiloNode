#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/test.sh

set -eu

cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/kilonode-compat replay-dir tests/fixtures/compat
./build/kilonode-compat replay-capture-dir tests/fixtures/compat
./build/kilonode-compat check-bench-pack tests/fixtures/bench/manifest.bench
./build/kilonode-compat replay-bench-pack tests/fixtures/bench/manifest.bench
./build/kilonode-compat bench-coverage tests/fixtures/bench/manifest.bench
./build/kilonode-compat check-bench-expected tests/fixtures/bench/ax25-diag-replay.expected
./build/kilonode-compat replay-bench-diagnostics tests/fixtures/bench/manifest.bench
./build/kilonode-compat check-prepared-expect tests/fixtures/bench/prepared-frames.expected
./build/kilonode-compat replay-bench-prepared tests/fixtures/bench/manifest.bench --expect tests/fixtures/bench/prepared-frames.expected
./build/kilonode-compat run-ax25-timer-replay-dir tests/fixtures/ax25-timer
./build/kilonode-compat run-ax25-timer-prepared-dir tests/fixtures/ax25-timer
./build/kilonode-compat run-ax25-loopback-dir tests/fixtures/ax25-loopback
./scripts/ax25-connect-dry-run-fixtures.sh
./scripts/m1-docs-audit.sh
./scripts/ax25-no-transmit-check.sh
./scripts/external-modem-check-configs.sh
./scripts/ax25-live-scheduler-smoke-check.sh
./scripts/ax25-safety-check.sh
./scripts/ax25-prepared-gate-report.sh
./scripts/ax25-response-bench-preflight.sh
manual_workspace="/tmp/kilonode-manual-captures-test-$$"
./build/kilonode-compat manual-workspace-init "$manual_workspace"
./build/kilonode-compat manual-workspace-check "$manual_workspace"
./build/kilonode-compat manual-import tests/fixtures/manual-captures/import-source/kiss-manual-sabm.capture --workspace "$manual_workspace" --notes "test import"
./build/kilonode-compat manual-validate --workspace "$manual_workspace"
./build/kilonode-compat manual-replay-all --workspace "$manual_workspace"
./build/kilonode-compat manual-summary --workspace "$manual_workspace"
./build/kilonode-compat check-pack tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat pack-coverage tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat replay-pack tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat check-requirements tests/fixtures/compat/linbpq-node/requirements.plan
./build/kilonode-compat requirements-coverage tests/fixtures/compat/linbpq-node/requirements.plan tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat check-command-profiles tests/fixtures/compat/linbpq-node/command-profiles.plan
./build/kilonode-compat risk-report tests/fixtures/compat/linbpq-node/requirements.plan
./scripts/compat-check-node-plan.sh
./build/kilonode-compat check-observation tests/fixtures/compat/blackbox-node-help.observation
./build/kilonode-compat check-observation tests/fixtures/compat/blackbox-node-info.observation
./scripts/bench-rx-check-configs.sh
