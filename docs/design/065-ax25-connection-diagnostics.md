# AX.25 Connection Diagnostics

M1.35 adds formatting helpers for AX.25 connection-table diagnostics. These
helpers are for tests and future read-only control-plane work. They do not add
control socket commands in this pass.

## Diagnostic Data

The diagnostics layer can format:

| Item | Output |
|------|--------|
| Connection key | Port, local endpoint, remote endpoint, and path. |
| Connection state | State, sequence values, retries, busy flag, and retransmit flag. |
| Action list | Last state-machine action intents. |
| Frame plans | Last generated action-to-frame plans. |
| Table summary | Record count and configured maximum. |
| Record summary | Key, state, last event, action count, plan count, and errors. |

## Safety

Diagnostic output is bounded by caller-provided buffers. Truncation returns a
deterministic buffer error. Unsafe keys are rejected by the key validator before
formatting. The formatter does not read files, execute commands, open
transports, enqueue frames, or dispatch frames.

## Future Control Plane

A later pass may expose read-only connection diagnostics through the local
control socket. That future command should reuse these formatting helpers and
must keep row counts and output lengths bounded.
