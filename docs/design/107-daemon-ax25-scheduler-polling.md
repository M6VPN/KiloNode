# Daemon AX.25 Scheduler Polling

The daemon now calls `kn_daemon_ax25_scheduler_poll(runtime, now_ms)` when the
AX.25 live scheduler policy is enabled. Tests pass injected monotonic
millisecond values. The daemon loop uses its existing monotonic wrapper to pass
the current time into the helper.

The helper has two paths:

| Mode       | Behaviour |
|------------|-----------|
| smoke off  | Polls the existing live scheduler wrapper. |
| smoke on   | Polls through the smoke helper, records smoke counters, and keeps TX blocked. |

No OS timers, threads, alarms, signals, or sleeps are introduced by this pass.
The scheduler remains explicitly polled by the daemon loop or tests.

`live-scheduler-process-expired false` means the poll updates scheduler status
without processing expired timers. `live-scheduler-process-expired true` allows
bounded expiry processing up to `live-scheduler-max-expired-per-cycle`.

Generated actions and prepared frames stay diagnostic-only. No queue bridge or
dispatch call is made by the daemon helper.
