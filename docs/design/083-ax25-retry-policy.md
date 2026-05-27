# AX.25 Retry Policy

M1.42 adds a bounded retry helper for N2 accounting. The helper tracks the
current retry count and the configured retry limit from AX.25 parameters.

Supported operations:

- validate max retry count
- reset retry count
- increment retry count
- check whether retry count remains under the limit
- check whether retry count is exhausted
- format deterministic diagnostics

The state machine already updates the connection retry count when T1 timeout
events are processed. The scheduler side table mirrors retry action intents for
future runtime diagnostics and tests.

Retry action handling has no TX side effects. A retransmission action intent is
diagnostic state only until a later milestone adds the action-plan to TX queue
bridge.

Limitations:

- No live retransmission scheduler is active.
- No outstanding I-frame queue exists.
- No modulo-128 retry/window policy is implemented.
