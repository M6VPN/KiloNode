# AX.25 Timer Model

M1.42 adds logical AX.25 timers for unit tests and future scheduler work.
The timers model T1, T2, and T3 as AX.25 connected-mode state data only.

| Timer | Purpose in scaffold | M1.42 status |
|-------|---------------------|--------------|
| T1    | Acknowledgement and retry timeout for setup/release waits. | Implemented as a logical timer. |
| T2    | Delayed acknowledgement timer. | Scaffolded as a logical timer with planned expiry handling. |
| T3    | Idle keepalive or poll timer. | Implemented as a logical timer that maps to the existing timeout event. |

Timer state tracks:

- kind
- running flag
- connection diagnostic ID
- start time in injected monotonic milliseconds
- expiry time
- duration
- generation counter
- expiry count

All timer checks take `now_ms` from the caller. Timer code does not call
`time()`, does not use signals, and does not start threads or operating-system
timers.

Invalid durations are rejected. The scaffold uses bounded integer checks before
calculating expiry times.

This pass does not add live daemon scheduling. Runtime and daemon paths do not
poll these timers automatically.
