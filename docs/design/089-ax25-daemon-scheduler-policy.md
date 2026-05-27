# AX.25 Daemon Scheduler Policy

M1.44 extends the optional `ax25` config block with scheduler boundary keys.

Safe defaults:

```text
live-scheduler false
live-scheduler-process-expired false
live-scheduler-max-expired-per-cycle 4
live-scheduler-tx-actions false
```

Policy fields:

| Field | Meaning |
|-------|---------|
| `enabled` | Allows the daemon-owned scheduler wrapper to report live status. |
| `process_expired` | Allows explicit poll calls to process expired logical timers. |
| `max_expired_per_cycle` | Bounds expiry processing per poll call. |
| `tx_actions_enabled` | Must remain false in M1.44. |
| `diagnostics_enabled` | Allows read-only scheduler diagnostics. |

Rejected combinations:

- `live-scheduler true` while `ax25 enabled false`.
- `live-scheduler-process-expired true` while `live-scheduler false`.
- `live-scheduler-tx-actions true`.
- `live-scheduler-max-expired-per-cycle` outside the bounded range.

Even when `live-scheduler true`, generated transmit-like actions are blocked and
counted. No TX queue write is attempted.

Future work may add a controlled action-plan to TX queue bridge, real
retransmission dispatch, RF CONNECT, and BBS session binding. Those are not part
of M1.44.
