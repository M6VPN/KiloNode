# M1 Test Inventory

All listed automated checks are no-hardware checks. They do not require a real
TNC, radio, USB sound card, Dire Wolf, KiloTNC, external service, or LinBPQ
binary.

## Build and Unit Tests

Run:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Major test areas:

- Core: `test_buffer`, `test_config`, `test_access_policy`, `test_stats`.
- KISS and transports: `test_kiss`, `test_kiss_stream`, `test_transport*`.
- Control plane: `test_control_protocol`, `test_control_socket`.
- Node shell: `test_node_command*`, `test_node_shell*`.
- BBS and message store: `test_bbs_*`, `test_message*`.
- RX/heard/RF command: `test_rx_*`, `test_heard`, `test_rf_*`.
- TX policy and gates: `test_tx_*`, `test_daemon_tx`.
- AX.25 primitives: `test_ax25`, `test_ax25_control`,
  `test_ax25_frame_builder`, `test_ax25_frame_plan`, `test_ax25_sequence`.
- AX.25 connected diagnostics: `test_ax25_state`,
  `test_ax25_connection*`, `test_ax25_runtime`, `test_ax25_rx_feed`.
- AX.25 timers/scheduler: `test_ax25_timer*`, `test_ax25_retry`,
  `test_ax25_scheduler*`, `test_daemon_ax25_scheduler`.
- Prepared diagnostics: `test_ax25_prepared*`.
- Bench/manual/compat: `test_bench_*`, `test_manual_capture_*`,
  `test_compat_*`.
- FX.25 scaffold: `test_fx25`, `test_fx25_params`.

## Scripts

Primary M1 scripts:

```sh
./scripts/test.sh
./scripts/m1-readiness-check.sh
./scripts/m1-docs-audit.sh
./scripts/m1-test-inventory.sh
./scripts/m1-safety-audit.sh
./scripts/m1-compatibility-audit.sh
```

Bench and AX.25 replay scripts:

```sh
./scripts/bench-rx-replay-fixtures.sh
./scripts/bench-rx-replay-diagnostics.sh
./scripts/ax25-timer-replay-fixtures.sh
./scripts/ax25-prepared-replay-fixtures.sh
./scripts/ax25-live-scheduler-smoke-check.sh
```

Safety scripts:

```sh
./scripts/ax25-no-transmit-check.sh
./scripts/ax25-safety-check.sh
./scripts/ax25-prepared-gate-report.sh
./scripts/ax25-response-bench-preflight.sh
```

Compatibility scripts:

```sh
./scripts/compat-validate-observation-pack.sh
./scripts/compat-check-node-plan.sh
./scripts/compat-replay-fixtures.sh
./scripts/compat-validate-captures.sh
```

## Fixture Directories

- `tests/fixtures/compat`: transcripts, captures, observations, and plans.
- `tests/fixtures/bench`: receive-only bench captures and expected files.
- `tests/fixtures/ax25-timer`: timer replay scripts and prepared expectations.
- `tests/fixtures/manual-captures`: import-source captures and workspace seed.
- `tests/fixtures/safety`: safety check requirements and blockers.
- `tests/fixtures/milestones`: M1 readiness requirements and blocked features.

## Manual-Only Work

- Real USB sound card, Dire Wolf, KiloTNC, serial TNC, or TCP KISS capture.
- Manual review before committing imported bench captures.
- Future black-box LinBPQ observations.
- Any future real TX lab work.

## Must Not Run In CI

- LinBPQ binary execution.
- GPL source inspection.
- Real TNC, serial, audio, or radio device access.
- Real TX dispatch.
- Daemon tests that depend on external services or real devices.
