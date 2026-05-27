# FX.25 Capture Placeholders

FX.25 decode and Reed-Solomon/FEC correction are not implemented in KiloNode.
The committed bench fixture pack includes `fx25-future-placeholder.capture` only
to keep FX.25 visible in receive-side bench planning.

Future FX.25 captures should include:

- known-good FX.25 frames
- corrected frames with documented correction counts
- uncorrectable frames
- embedded AX.25 payload extraction cases
- fallback to normal AX.25 when no FX.25 layer is present
- interoperability notes for non-FX.25 stations

Until FX.25 support exists, FX.25 placeholders must not be treated as decoded
packet support.
