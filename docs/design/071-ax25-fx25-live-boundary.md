# AX.25 and FX.25 Live Boundary

The M1.37 live diagnostics feed sits after AX.25 decode. It receives decoded
AX.25 frames and records diagnostic state in the AX.25 runtime.

FX.25 remains a separate layer. Future FX.25 receive support should detect and
decode FX.25 first, then pass recovered AX.25 frame bytes into the existing
AX.25 decoder. Only decoded AX.25 frames should reach the connection diagnostic
feed.

Future FX.25 transmit support should wrap already-built AX.25 response frames
after action-to-frame mapping and before the final physical or TNC framing
layer.

M1.37 does not implement FX.25 detection, Reed-Solomon correction, FEC encode,
or live FX.25 integration.
