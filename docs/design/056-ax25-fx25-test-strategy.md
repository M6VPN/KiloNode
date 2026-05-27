# AX.25 and FX.25 Test Strategy

The specs under `docs/references` are the primary reference set for AX.25 and
FX.25 planning. This pass adds scaffold tests only.

Current synthetic tests:

| Area           | Test focus |
|----------------|------------|
| AX.25 control  | Classify UI, I, S, and U frames, including S/U subtype placeholders. |
| AX.25 params   | Validate disabled defaults and reject invalid timers, windows, retries, and lengths. |
| AX.25 state    | Validate init/reset, event mapping, formatting, and no TX output. |
| FX.25 params   | Validate disabled defaults and detect-only parameters. |
| FX.25 layer    | Verify placeholders do not claim FEC, correction, or AX.25 extraction. |

Future test vectors needed:

- AX.25 I-frame examples for modulo 8.
- AX.25 extended control examples for modulo 128.
- Supervisory and unnumbered frame examples with poll/final variants.
- Connected-mode trace captures for setup, transfer, release, and timeout cases.
- FX.25 correlation tag examples.
- FX.25 FEC valid, corrected, and uncorrectable examples.
- AX.25 fallback examples where no FX.25 wrapper is present.

Future black-box captures:

- KISS packet-boundary captures from lab-only systems.
- AXIP/AXUDP captures when those modes are introduced.
- Connected-mode observations recorded as external behaviour only.

No test in this pass uses real TNC hardware, starts LinBPQ, dispatches RF, or
requires external services.
