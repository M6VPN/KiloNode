# AX.25 Runtime Integration Boundary

M1.36 adds a daemon-owned AX.25 runtime scaffold. The runtime is initialized by `kilonoded`, exposed read-only through the control socket, and cleaned up on shutdown.

## Runtime Fields

The runtime stores:

| Field | Purpose |
|-------|---------|
| `enabled` | Global AX.25 runtime flag. Defaults false. |
| `connected_mode_enabled` | Connected-mode processing flag. Defaults false. |
| `diagnostics_enabled` | Allows read-only status reporting. Defaults true. |
| `max_connections` | Bounded connection-table size. Defaults 32. |
| `params` | AX.25 connected-mode parameter snapshot. |
| `table` | Fixed-size connection table. |
| `counters` | Diagnostic event and frame-plan counters. |

## Disabled Default

The runtime exists by default, but live RX processing is not wired into it. `enabled` and `connected_mode_enabled` both default false.

Unit tests can inject decoded AX.25 connection events into the runtime. That helper is not exposed through the shell, RF UI commands, BBS shell, or control socket.

## No Live RX Feed

The daemon still decodes KISS and AX.25 UI traffic for the existing receive, heard, and RF command paths. Connected-mode frames are not fed into the runtime from live daemon RX in this pass.

This keeps the scaffold observable without enabling connected sessions or accidental responses.

## No TX Bridge

The AX.25 state machine may produce action intents, and the action mapper may produce frame plans. The runtime stores those plans for diagnostics only.

No runtime code calls:

- `kn_tx_queue_enqueue`
- TX dispatcher functions
- transport write functions
- shell or BBS payload delivery functions

## Future Integration Path

Deferred work:

- live RX event feed into the connection table
- action-plan to TX queue bridge
- timer scheduler for T1/T2/T3
- RF `CONNECT`
- connected-mode node shell binding
- BBS session binding
- read-only control counters for live frame intake

## FX.25 Boundary

FX.25 remains a separate layer. Future FX.25 receive work should decode or reject FX.25 frames before AX.25 connection event creation. Future FX.25 transmit work should wrap already-built AX.25 frame bytes after the AX.25 action-to-frame mapper.

No FX.25 FEC encode/decode is implemented in M1.36.
