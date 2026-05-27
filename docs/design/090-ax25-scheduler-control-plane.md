# AX.25 Scheduler Control Plane

M1.44 adds read-only AX.25 scheduler diagnostics to the daemon control plane and
`kilonodectl`.

Commands:

```text
AX25 SCHEDULER
AX25 SCHEDULER TIMERS
AX25 SCHEDULER COUNTERS
```

`AX25 SCHEDULER` returns policy and next wakeup state:

```text
OK AX25 SCHEDULER enabled=false process_expired=false max_expired=4 tx_actions=false next_wakeup=0
END
```

`AX25 SCHEDULER TIMERS` returns bounded timer rows:

```text
OK AX25 SCHEDULER TIMERS count=0
END
```

`AX25 SCHEDULER COUNTERS` returns scheduler diagnostics:

```text
OK AX25 SCHEDULER COUNTERS cycles=0 expired=0 blocked=0 plans=0 tx_blocked=0 tx_writes=0
END
```

There are no mutation commands. The control plane cannot start or stop timers,
process timers, create connections, enqueue TX frames, or dispatch RF frames.

`kilonodectl` commands:

```text
kilonodectl ax25 scheduler
kilonodectl ax25 scheduler-timers
kilonodectl ax25 scheduler-counters
```
