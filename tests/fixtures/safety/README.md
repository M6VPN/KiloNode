# Safety Fixtures

These fixtures describe the current AX.25 response safety gate in a simple
line-based format for shell checks.

`ax25-response-safety.required` lists checks that must pass before the current
non-transmitting safety gate is considered healthy.

`ax25-response-safety.blocked` lists blockers that prevent real connected-mode
AX.25 response TX.

`ax25-response-safety.report.expected` contains deterministic output snippets
used by the safety scripts.
