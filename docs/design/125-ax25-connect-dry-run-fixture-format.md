# AX.25 CONNECT Dry-Run Fixture Format

CONNECT dry-run fixtures use `.connectdry` files.

Example:

```text
# KiloNode AX.25 CONNECT dry-run v1
name basic
local M6VPN-1
remote N0CALL
port kiss0
params modulo=8 window=1 paclen=256 t1=3000 t2=1000 t3=300000 n2=3
expect planned-state awaiting-connection
expect frame-kind SABM
expect bridge blocked
expect connection-created false
expect tx-writes 0
expect dispatch-calls 0
expect fx25-frames 0
```

Rules:

- comments start with `#`
- one command per line
- line length is bounded
- local and remote callsigns must parse as AX.25 callsigns
- port names are bounded
- params must pass AX.25 param validation
- unknown commands and expectations fail parsing
- expectation mismatches fail the run

The format has no shell commands, no path includes, and no transport open.
