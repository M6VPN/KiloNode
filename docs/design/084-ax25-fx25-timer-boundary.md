# AX.25 and FX.25 Timer Boundary

AX.25 T1, T2, T3, and N2 belong to the AX.25 connected-mode layer.

FX.25 remains separate. Future FX.25 receive work may detect and decode an
FX.25 block before AX.25 frame decode. Future FX.25 transmit work may wrap an
already-built AX.25 frame after AX.25 state and action mapping.

The M1.42 scheduler does not inspect FX.25 tags, FEC state, correction status,
or block timing. It only accepts AX.25 connection action intents and AX.25 timer
expiry events.

No FX.25 FEC encode or decode is implemented in this pass.
