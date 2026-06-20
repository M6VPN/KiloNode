# AX.25 CONNECT Dry-Run Safety Boundary

The CONNECT dry-run planner is not a live CONNECT path. It does not alter daemon
state and does not enqueue or dispatch frames.

The following remain blocked:

- node shell CONNECT
- BBS shell CONNECT
- RF command CONNECT
- control socket mutation
- connection table insertion
- real TX queue writes
- TX dispatch
- prepared-to-TX bridge promotion
- RF BBS access
- NET/ROM routing
- FX.25 wrapping or FEC

Dry-run reports must keep `connection_created=false`, `bridge=blocked`,
`tx_writes=0`, `dispatch=0`, and `fx25=0`.
