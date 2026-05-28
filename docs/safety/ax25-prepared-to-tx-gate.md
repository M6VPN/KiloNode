# AX.25 Prepared-to-TX Gate

The prepared-to-TX gate evaluates prepared AX.25 diagnostics without writing to
the real TX queue. The bridge remains disabled in runtime. Passing these checks
does not transmit.

| Gate | Default | Required future value | Proof required | Current status |
|------|---------|-----------------------|----------------|----------------|
| Bridge enabled | `false` | explicit operator config | config tests and safety review | blocked |
| Test-only mode | `true` | reviewed transition plan | separate TX milestone | test-only |
| Control frames | `false` | explicit control-frame allow | replay plus bench gate | blocked |
| I frames | `false` | explicit data policy | I-frame state and payload tests | planned |
| TX policy enabled | required | true | TX policy tests | not active |
| Port TX enabled | required | true | per-port gate tests | not active |
| Dispatch disabled | required | reviewed dispatch policy | dispatch safety tests | blocked |
| No auto-dispatch | required | separate design | operator preflight | blocked |
| Max per call | `4` | bounded value | stress and queue tests | diagnostic |
| FX.25 wrapping | `false` | separate FX.25 checklist | FEC and wrapper tests | blocked |

The gate can report why a prepared frame would not pass: bridge disabled,
missing raw AX.25 bytes, disabled control frames, disabled TX policy, disabled
port TX, dispatch enabled, auto-dispatch blocked, or FX.25 unsupported.
