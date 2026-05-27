# AX.25 Action Model

The M1.33 state machine returns bounded action intents. Intents describe what
future integration code should do, but they do not create frame bytes, enqueue
TX frames, write transports, start timers, or call session code.

Action groups:

| Group      | Intents |
|------------|---------|
| Frame      | send-sabm, send-sabme, send-ua, send-dm, send-disc, send-rr, send-rnr, send-rej, send-frmr |
| Delivery   | deliver-i-payload |
| Timers     | start-t1, stop-t1, start-t3, stop-t3 |
| Retry      | reset-retry-count, increment-retry-count |
| State note | enter-connected, enter-disconnected |
| Error      | protocol-error, retransmit-needed |

Rules:

- Action lists are bounded.
- Overflow is a deterministic error.
- Formatting is deterministic for unit tests and diagnostics.
- Sequence-bearing actions carry only a small sequence value.
- No action contains AX.25 frame bytes or payload bytes.

Future mapping:

A later integration layer may translate action intents into outbound AX.25
frames behind existing TX policy and safety gates. That layer must remain
separate from the state machine and must not bypass dispatch gates.
