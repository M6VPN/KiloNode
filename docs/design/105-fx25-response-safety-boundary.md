# FX.25 Response Safety Boundary

FX.25 remains outside AX.25 response TX safety in this milestone.

The AX.25 safety checklist covers AX.25 frame plans and prepared AX.25 bytes.
FX.25 wrapping would be a later stage after AX.25 generation and before final
physical or TNC framing.

Future FX.25 response work needs its own proof for FEC encode, frame sizing,
fallback behaviour, and interop. It must not alter AX.25 timer behaviour or
connected-mode retry accounting.
