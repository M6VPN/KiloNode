# AX.25 Prepared TX Policy

The prepared TX policy describes bridge decisions without enabling real
transmit.

Safe defaults:

| Field | Default | Meaning |
|-------|---------|---------|
| `bridge_enabled` | `false` | The bridge gate is off. |
| `test_only` | `true` | Only test-only conversion is allowed. |
| `allow_control_frames` | `false` | Control frames are not bridgeable by default. |
| `allow_i_frames` | `false` | I-frame bridging is planned only. |
| `require_tx_policy_enabled` | `true` | A future bridge must see TX policy enabled. |
| `require_port_tx_enabled` | `true` | A future bridge must see the port TX gate enabled. |
| `require_dispatch_disabled` | `true` | Dispatch must remain off for this gate. |
| `require_no_auto_dispatch` | `true` | Automatic dispatch is not allowed. |
| `max_bridge_per_call` | `4` | Bounded future batch size. |
| `allow_fx25_wrapping` | `false` | FX.25 wrapping is not part of this gate. |

Rejected combinations:

- `bridge_enabled=true` with `test_only=false`
- `allow_i_frames=true`
- `allow_fx25_wrapping=true`
- invalid boolean values
- `max_bridge_per_call` outside the documented bounds

Gate reasons are deterministic and include disabled bridge, missing raw AX.25
bytes, disabled control frames, disabled TX policy, disabled port TX, dispatch
enabled, unsupported FX.25, and allowed test-only.
