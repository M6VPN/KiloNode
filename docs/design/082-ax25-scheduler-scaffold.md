# AX.25 Scheduler Scaffold

M1.42 adds an offline scheduler scaffold around AX.25 action intents. It owns a
bounded timer queue and a small retry side table keyed by diagnostic connection
ID.

The scheduler can apply these state-machine action intents:

| Action intent           | Scheduler effect |
|-------------------------|------------------|
| `start-t1`              | Start or restart logical T1. |
| `stop-t1`               | Stop logical T1 if present. |
| `start-t3`              | Start or restart logical T3. |
| `stop-t3`               | Stop logical T3 if present. |
| `reset-retry-count`     | Reset the logical N2 retry helper. |
| `increment-retry-count` | Increment the logical N2 retry helper. |
| `send-*`                | No scheduler effect and no TX queue write. |

Expiry collection is deterministic:

1. Earlier expiry time first.
2. Lower connection ID next.
3. Lower timer kind last.

T1 expiry maps to `timeout-t1`. T3 expiry maps to `timeout-t3`. T2 expiry is
reported as planned because the state machine does not yet expose delayed-ACK
behaviour.

The scheduler can feed a collected expiry into an existing AX.25 connection
object and retain the resulting action list for tests. It does not build
outbound frames, does not enqueue frame plans, does not dispatch transports, and
does not integrate with the daemon event loop in this pass.
