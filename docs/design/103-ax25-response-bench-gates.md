# AX.25 Response Bench Gates

Bench validation is staged so that offline and receive-only evidence comes
before any TX lab work.

Stage 0 uses deterministic fixtures and replay scripts only. It requires no
hardware and must show TX writes remain zero.

Stage 1 observes real receive-only sources through KISS. TX remains blocked,
PTT is not connected, and diagnostics are compared with logs and captures.

Stage 2 is a future dummy-load or non-radiating TX lab. It requires explicit TX
policy, per-port TX, operator controls, and a reviewed stop path.

Stage 3 is a future controlled over-air legal test. It requires Stage 2 proof,
legal authority, and reviewed logs.
