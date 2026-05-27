# AX.25 Prepared To TX Boundary

Prepared AX.25 frames are diagnostics only. They sit after the state machine, action mapper, and frame builder, but before any future TX queue bridge.

The future bridge to the real TX queue needs separate safety gates:

| Gate | Requirement |
|------|-------------|
| Operator policy | TX must be explicitly enabled by config and operator action. |
| Per-port TX | The selected port must allow TX. |
| Callsign policy | Local callsign and legal-use checks must pass. |
| Scheduler policy | Retry and timer behaviour must be enabled intentionally. |
| Channel policy | Channel/PTT/dispatch rules must permit transmit. |
| Dispatch policy | Real dispatch gates must be enabled. |

This pass implements none of those TX behaviours. `prepared-bridge-to-tx true` is rejected, and the bridge helper returns blocked.
