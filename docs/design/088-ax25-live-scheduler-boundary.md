# AX.25 Live Scheduler Boundary

M1.44 adds a disabled-by-default daemon boundary for AX.25 scheduler
diagnostics.

The daemon can own a live scheduler wrapper around the existing logical AX.25
scheduler. The wrapper exposes policy, timer rows, and counters through the
read-only control plane. It does not enqueue frames, dispatch frames, expose a
CONNECT command, or bind sessions to shell or BBS code.

The live boundary is explicit:

1. AX.25 runtime owns the existing logical scheduler.
2. AX.25 runtime owns a live scheduler policy wrapper.
3. The daemon may call `kn_ax25_live_scheduler_poll(runtime, now_ms)` only when
   the live scheduler policy is enabled.
4. Tests inject `now_ms`; daemon polling uses a small monotonic-time wrapper.
5. Generated transmit-like actions are counted as blocked diagnostics.

There are no operating-system timers, threads, alarms, signals, or sleeps in
the scheduler code. Polling is explicit and bounded by
`live-scheduler-max-expired-per-cycle`.

This boundary relates to the offline timer replay harness from M1.43. The replay
harness proves scheduler and timeout behaviour with injected time. M1.44 only
adds daemon ownership and read-only visibility.
