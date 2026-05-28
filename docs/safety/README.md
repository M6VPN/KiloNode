# KiloNode Safety

KiloNode does not enable connected-mode AX.25 response TX in the current
milestone. Real TX remains blocked, prepared response frames are diagnostics,
and the prepared-to-TX bridge is a future gate.

This section defines proof required before any future milestone can promote
prepared AX.25 responses into the real TX queue. Passing these documents or
checks is not permission to transmit.

Start here:

- [AX.25 response safety checklist](ax25-response-safety-checklist.md)
- [Prepared-to-TX gate](ax25-prepared-to-tx-gate.md)
- [AX.25 response bench gate](ax25-response-bench-gate.md)
- [Operator preflight](ax25-operator-preflight.md)
- [No-transmit regression](ax25-no-transmit-regression.md)
- [FX.25 safety placeholders](fx25-safety-placeholders.md)
