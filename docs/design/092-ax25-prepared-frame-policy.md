# AX.25 Prepared Frame Policy

The prepared-frame diagnostics policy controls whether generated frame plans are copied into the diagnostics queue and whether raw AX.25 bytes are built.

Config keys:

| Key                   | Default | Purpose |
|-----------------------|---------|---------|
| `prepared-frames`     | `true`  | Store generated frame plans in the diagnostics queue. |
| `prepared-max-frames` | `128`   | Bound the diagnostics queue. |
| `prepared-build-raw`  | `true`  | Build bounded raw AX.25 bytes for inspection. |
| `prepared-bridge-to-tx` | `false` | Reserved for a future TX bridge. `true` is rejected. |

Prepared diagnostics may be enabled while AX.25 live CONNECT and real TX remain disabled. Build failures are stored as diagnostic counters and do not alter state-machine behaviour.

The bridge helper returns a blocked result in this pass. It does not call TX queue or dispatch code.
