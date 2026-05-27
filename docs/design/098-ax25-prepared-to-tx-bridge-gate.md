# AX.25 Prepared-to-TX Bridge Gate

M1.47 adds a diagnostic gate between the AX.25 prepared-frame queue and the
real transmit queue.

The gate evaluates whether a prepared AX.25 response frame has enough safe
state to become a future TX frame. It does not promote frames into the daemon
TX queue, does not dispatch, and does not start retransmission.

The runtime policy defaults to disabled. Read-only control commands can show
the policy, per-frame gate decisions, and counters.

The test-only conversion helper can build an in-memory `kn_tx_frame` from a
prepared AX.25 frame after every gate passes. That helper is for unit tests and
diagnostics. The daemon runtime path still returns blocked.

Future work must add a separate milestone for real queue promotion with
operator policy, per-port TX enablement, dispatch gates, and legal/callsign
checks.
