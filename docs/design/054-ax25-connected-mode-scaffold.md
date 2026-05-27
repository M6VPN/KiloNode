# AX.25 Connected-Mode Scaffold

M1.32 creates compile-tested connected-mode data shapes without enabling
connected-mode behaviour.

Runtime pieces:

| Module            | Contents |
|-------------------|----------|
| `ax25_control`    | Control byte classification and subtype naming. |
| `ax25_params`     | Disabled future parameters for timers, retries, modulo mode, windows, and information length. |
| `ax25_connection` | State and event enums, reset/init helpers, formatting, and validation. |

Defaults:

- Connected mode is disabled.
- A new connection scaffold starts in `disabled`.
- TX output from the connection layer is not allowed.
- No transition table is present.

Future milestones:

1. RX mapping from decoded AX.25 frames to connection events.
2. A connection table keyed by port, source, and destination.
3. Timer scheduling for T1, T2, and T3.
4. Modulo 8 sequence handling.
5. Modulo 128 sequence handling after the modulo 8 model is tested.
6. Response frame generation behind existing TX gates.
7. RF node shell and BBS binding after command/session policy exists.

Out of scope:

- CONNECT command support.
- Automatic UA, DM, RR, RNR, REJ, or DISC generation.
- NET/ROM routing.
- RF BBS access.
