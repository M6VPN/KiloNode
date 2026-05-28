# AX.25 Live Scheduler Smoke Mode

M1.49 adds a disabled-by-default smoke mode for the live AX.25 scheduler. It is
diagnostics-only plumbing for proving that the daemon can own the AX.25 runtime,
poll the logical scheduler, and expose counters through the control plane.

Smoke mode is gated by:

```text
ax25 {
	enabled true
	diagnostics true
	live-scheduler true
	live-scheduler-smoke true
	live-scheduler-smoke-create-test-connection false
	live-scheduler-tx-actions false
}
```

`live-scheduler-smoke-create-test-connection` is also disabled by default. When
enabled in a diagnostic config, the helper creates one synthetic connection on
port `smoke0` from `M6VPN-1` to `N0CALL`, starts logical timers through normal
state-machine action side effects, and retains prepared frame diagnostics.

Smoke mode never promotes prepared frames to the real TX queue. The prepared
bridge is only probed through the existing blocked runtime helper so diagnostics
can prove the bridge remains closed.

Counters include cycles, synthetic test connections, scheduler polls, expired
timers, prepared frames, bridge-blocked checks, TX write attempts, and dispatch
attempts. TX writes and dispatch attempts must remain zero.
