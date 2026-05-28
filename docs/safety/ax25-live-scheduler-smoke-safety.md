# AX.25 Live Scheduler Smoke Safety

AX.25 live scheduler smoke mode is diagnostics only. It can prove that the
daemon owns an AX.25 runtime and can poll logical timers, but it is not TX
readiness proof.

Safe defaults:

- `live-scheduler-smoke false`
- `live-scheduler-smoke-create-test-connection false`
- `live-scheduler-tx-actions false`
- `prepared-bridge-to-tx false`
- transmit dispatch disabled in default examples

If smoke mode is enabled in a local diagnostic config, confirm:

- `ax25.enabled true`
- `ax25.diagnostics true`
- `ax25.live-scheduler true`
- `ax25.live-scheduler-tx-actions false`
- `ax25.prepared-bridge-to-tx false`
- transmit dispatch remains disabled
- port `tx-enabled` remains false unless a future lab-only TX milestone changes that policy

Smoke mode may create one synthetic diagnostic connection only when
`live-scheduler-smoke-create-test-connection true` is set. That connection is
not bound to shell, BBS, RF commands, NET/ROM, or a real TX queue.

FX.25 remains separate and planned. Smoke mode does not wrap frames, run FEC, or
change AX.25 timer behaviour based on FX.25 state.
