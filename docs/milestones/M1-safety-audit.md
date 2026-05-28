# M1 Safety Audit

M1 is receive-side and diagnostics focused. Real connected-mode response TX is
blocked.

## No-Transmit Defaults

Default and receive-only example configs keep:

- `transmit.enabled false`
- `transmit.dispatch-enabled false`
- `transmit.dispatch-real-kiss false`
- per-port `tx-enabled false`
- `ax25.prepared-bridge-to-tx false`
- `ax25.live-scheduler-tx-actions false`

Run:

```sh
./scripts/ax25-no-transmit-check.sh
```

## TX Gates

TX queue, TX policy, transport gates, dry-run UI, and dispatch have tests. Real
dispatch requires explicit policy and per-port gates. M1 readiness does not
enable those gates in default or receive-only configs.

## Prepared-to-TX Bridge

Prepared AX.25 frames are diagnostics only. The runtime bridge helper returns a
blocked result and increments diagnostics counters without writing to the real
TX queue.

Run:

```sh
./scripts/ax25-prepared-gate-report.sh
```

## AX.25 Scheduler Smoke Mode

Smoke mode is disabled by default. When enabled in a diagnostic config, it polls
logical AX.25 timers and may create one synthetic diagnostic connection. It does
not dispatch, promote prepared frames, or write to the real TX queue.

Run:

```sh
./scripts/ax25-live-scheduler-smoke-check.sh
```

## Bench Configs

Receive-only bench configs are checked by:

```sh
./scripts/bench-rx-check-configs.sh
```

They keep TX disabled and port TX disabled.

## Scripts

M1 scripts are intended to run without elevated privileges, hardware, external
services, or real device access. They do not run LinBPQ.

## Blocked Features

`tests/fixtures/milestones/m1-blocked-features.txt` records features that must
remain blocked for the alpha tag, including CONNECT, real response TX, RF BBS,
NET/ROM, forwarding, FX.25 FEC, PTT control, and channel-busy policy.
