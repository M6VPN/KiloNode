# AX.25 and FX.25 I-Frame Boundary

M2.2 I-frame logic operates on AX.25 frame bytes only.

FX.25 remains outside this path:

- no FX.25 FEC encode
- no FX.25 FEC decode
- no FX.25 wrapping
- no FX.25 timer or sequence coupling
- no FX.25 loopback frame generation

Future FX.25 loopback work may wrap raw AX.25 I frames after the AX.25 frame is
built and unwrap before AX.25 decode. That future work needs separate vectors
and safety checks.

Current loopback reports keep:

```text
fx25=0
```

M2.3 segmentation does not change this boundary. Segmentation splits AX.25
I-frame payload bytes before raw AX.25 frame generation; FX.25 wrapping remains
a future stage around complete AX.25 frames.
