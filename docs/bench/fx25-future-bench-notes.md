# FX.25 Future Bench Notes

FX.25 support is not implemented in M1.38.

Future FX.25 bench validation should check:

- FX.25 frame detection.
- FEC correction statistics.
- AX.25 payload extraction.
- Fallback to normal AX.25 when frames are not FX.25.
- Compatibility with non-FX.25 stations.

The current receive bench validates decoded KISS/AX.25 input only. FX.25 decode
would happen before AX.25 live diagnostics in a later milestone.
