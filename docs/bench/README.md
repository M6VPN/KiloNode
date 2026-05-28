# Receive-Only Bench Validation

This bench pack is for receive-only AX.25 validation. It feeds KISS data into
KiloNode, checks decoded RX state, and checks AX.25 live diagnostic counters.
It does not enable transmit.

Supported input paths:

- Dire Wolf TCP KISS from a USB sound card
- KiloTNC KISS, when available
- Serial KISS TNC
- TCP KISS
- PTY KISS
- Unix socket KISS

Synthetic capture fixtures for this workflow live in `tests/fixtures/bench`.
See [capture-fixtures.md](capture-fixtures.md) and
[replay-bench-captures.md](replay-bench-captures.md). AX.25 diagnostic replay
is documented in [diagnostic-replay.md](diagnostic-replay.md).
Manual capture workspaces are documented in
[manual-capture-workspace.md](manual-capture-workspace.md),
[manual-capture-index.md](manual-capture-index.md), and
[manual-capture-replay.md](manual-capture-replay.md).
Future response TX bench gates are documented in
[ax25-response-bench-gate.md](ax25-response-bench-gate.md),
[ax25-prepared-response-validation.md](ax25-prepared-response-validation.md),
and [ax25-future-tx-lab-notes.md](ax25-future-tx-lab-notes.md). Live scheduler
smoke diagnostics are documented in
[ax25-live-scheduler-smoke.md](ax25-live-scheduler-smoke.md). Current bench work
remains offline or receive-only.

Primary validation commands:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --check-config
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --foreground
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 connections
./build/kilonodectl --socket /tmp/kilonode/control.sock rx events
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
```

Read [receive-only-safety.md](receive-only-safety.md) before connecting bench
hardware.

Milestone readiness checks are collected under
[docs/milestones](../milestones/M1-v0.1-alpha-readiness.md).
