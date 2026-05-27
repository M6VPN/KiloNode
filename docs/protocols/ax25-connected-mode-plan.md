# AX.25 Connected-Mode Plan

M1.32 adds planning scaffolding only. It does not add CONNECT, does not create
sessions over RF, and does not transmit connected-mode response frames.

Planned layers:

| Layer                  | Purpose | M1.32 status |
|------------------------|---------|--------------|
| Control classification | Identify I, S, U, and UI frame classes and named S/U subtypes. | Scaffold |
| Connection table       | Track local/remote endpoint state. | Planned |
| Timers                 | Model T1, T2, and T3 with bounded validation. | Scaffold |
| State machine          | Apply AX.25 connected-mode procedures. | Deferred |
| Retransmission         | Track N2, sequence numbers, and outstanding I frames. | Deferred |
| Segmentation           | Enforce future paclen/max-info policy. | Deferred |
| RF shell binding       | Bind a connected link to a node shell context. | Deferred |
| BBS session binding    | Bind a connected link to BBS access. | Deferred |

Current scaffold:

- `ax25_control` classifies control bytes and names frame classes/subtypes.
- `ax25_params` validates conservative, disabled-by-default future parameters.
- `ax25_connection` defines state/event structs and formatting helpers.

The scaffold produces no TX frames and is not called from the daemon RX loop.
