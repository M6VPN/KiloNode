# AX.25 FX.25 Prepared Frame Boundary

Prepared frames contain AX.25 frame plans and optional raw AX.25 bytes. FX.25 remains outside this queue.

Future FX.25 transmit work would wrap prepared AX.25 bytes after AX.25 frame generation and before final physical or TNC framing. Future FX.25 receive work would unwrap FX.25 before AX.25 decode and before connection diagnostics.

No FX.25 FEC, wrapping, or decode is implemented in this pass. Prepared-frame diagnostics only prove the AX.25 output boundary.
