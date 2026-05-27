# Prepared Frame Assertion Model

Prepared frame assertions compare offline replay output against expected
diagnostic frames. They inspect the AX.25 prepared diagnostics queue, not the
real TX queue.

Matching rules:

- `prepared-count` is exact.
- `prepared N` matches the Nth prepared frame in deterministic queue order.
- Missing frames fail.
- Extra frames fail when `prepared-count` is present.
- Wrong kind, action, status, callsign, port, length, or hex prefix fails.
- Any non-zero TX write count fails.

Bench capture replay uses the separate `prepared-frames.expected` file. Timer
replay uses inline expectations in `.replay` scripts and may also carry
separate expected files for review context.

FX.25 prepared expectations are not implemented. FX.25 wrapping remains a
future layer after AX.25 prepared bytes and before final physical or TNC
framing.
