# AX.25 and FX.25 Segmentation Boundary

M2.3 segmentation operates on AX.25 connected-mode I-frame payload bytes.

FX.25 is not part of the segmentation logic:

- no FX.25 FEC encode
- no FX.25 FEC decode
- no FX.25 wrapping
- no FX.25 sequence accounting
- no FX.25 paclen policy

The loopback simulator builds complete raw AX.25 I frames for each segment.
Future FX.25 work would wrap those complete AX.25 frames after AX.25 generation
and unwrap them before AX.25 decode.

Loopback reports must continue to show:

```text
fx25=0
```
