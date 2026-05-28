# Live Scheduler No-Transmit Proof

M1.49 keeps the live scheduler smoke mode inside the no-transmit boundary.

Required proof points:

- Real TX queue write attempts remain zero.
- Prepared-to-TX bridge checks remain blocked.
- Dispatch call attempts remain zero.
- No shell, BBS, RF command, or local command exposes CONNECT.
- FX.25 is not coupled to AX.25 timer scheduling.

The smoke control output exposes:

```text
AX25 SCHEDULER SMOKE
```

The response includes `tx_writes=0` and `dispatch_calls=0`. The general AX.25
counters also include smoke counters so tests can verify no transmit path was
touched.

The smoke check script is:

```text
./scripts/ax25-live-scheduler-smoke-check.sh
```

It runs smoke-mode tests when the build tree exists and then runs the
no-transmit configuration check. It does not start the daemon, open devices, run
external services, or use hardware.
