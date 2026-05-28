# FX.25 Safety Placeholders

FX.25 is not implemented in KiloNode. AX.25 prepared frames are stored as AX.25
bytes only, and FX.25 wrapping is blocked.

Future FX.25 TX work needs a separate checklist:

- [ ] FEC encode correctness.
- [ ] Fallback compatibility with non-FX.25 stations.
- [ ] Frame size and overhead limits.
- [ ] Decode and encode interop captures.
- [ ] No hidden coupling with AX.25 timers.
- [ ] Clear boundary after AX.25 frame generation and before final TNC framing.

No FX.25 wrapping is allowed in this pass.
