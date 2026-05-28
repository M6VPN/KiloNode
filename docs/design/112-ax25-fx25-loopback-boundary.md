# AX.25 and FX.25 Loopback Boundary

M2.1 loopback exchanges raw AX.25 frame bytes between two simulator endpoints.
It does not wrap frames in FX.25, decode FX.25, or run FEC.

The intended future layering is:

```text
AX.25 loopback frame bytes
future optional FX.25 wrap or unwrap boundary
future physical or TNC framing boundary
```

That future work must remain separate from the AX.25 state machine. AX.25 T1,
T2, T3, retry, prepared-frame, and loopback diagnostics continue to operate on
decoded AX.25 frames only.

Current loopback reports must show:

```text
fx25_frames=0
```

Any future FX.25 loopback milestone needs separate fixtures for wrapping,
decode, FEC success, FEC failure, fallback, frame-size limits, and interop.
