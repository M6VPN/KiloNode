# FX.25 Layer Model

M1.32 models FX.25 as a disabled protocol layer that may later detect,
decode, or encode FEC-wrapped AX.25 packet bytes.

Current structs and enums:

| Item                    | Purpose |
|-------------------------|---------|
| FX.25 mode              | Disabled, detect-only, encode-planned, or decode-planned. |
| Decode status           | not-fx25, candidate, valid, corrected, uncorrectable, malformed, or not-implemented. |
| Payload relation        | Unknown or embedded AX.25 frame bytes. |
| FEC profile             | none or Reed-Solomon planned. |
| Parameter struct        | Disabled-by-default validation for future RX/TX integration. |

Defaults:

- FX.25 is disabled.
- Detect-only is disabled.
- AX.25 fallback is allowed.
- Strict mode is disabled.

No FEC exists yet. The decode entry point is a placeholder. When FX.25 is
enabled and non-empty data is supplied, it returns a not-implemented result
instead of claiming decode support.

Future milestones:

1. Synthetic tag/correlation test vectors.
2. Offline capture decode reports.
3. Detect-only correlation matching.
4. AX.25 packet extraction from validated FX.25 frames.
5. Reed-Solomon decode.
6. Reed-Solomon encode.
7. KISS RX/TX path integration.
