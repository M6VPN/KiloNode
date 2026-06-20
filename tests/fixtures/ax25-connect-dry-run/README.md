# AX.25 CONNECT Dry-Run Fixtures

These fixtures exercise the M2.6 offline CONNECT dry-run planner. They validate
hypothetical local-admin CONNECT intent without creating connections, writing to
the real TX queue, dispatching frames, exposing CONNECT, or using FX.25.

Valid fixtures must report `tx_writes=0`, `dispatch=0`, `fx25=0`,
`connection_created=false`, and `bridge=blocked`.
