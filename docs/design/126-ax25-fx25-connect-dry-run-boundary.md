# AX.25 and FX.25 CONNECT Dry-Run Boundary

The M2.6 CONNECT dry-run planner evaluates AX.25 connected-mode setup intent
only. It does not create FX.25 frames and does not apply FEC or wrapping policy.

Future FX.25 work may wrap complete AX.25 frames after AX.25 frame generation.
That is a separate milestone with separate test vectors and safety checks.

Current dry-run fixtures must report `fx25=0`.
