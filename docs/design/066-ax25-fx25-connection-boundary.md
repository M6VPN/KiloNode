# AX.25 And FX.25 Connection Boundary

The AX.25 connection table operates on decoded AX.25 frames and generated
AX.25 frame plans only. FX.25 remains a separate scaffold and is not part of
the AX.25 connection state machine, event wrapper, table, or action mapper.

## RX Boundary

Future FX.25 receive support should run before AX.25 connection processing:

1. Receive packet bytes from a transport or capture boundary.
2. Optionally detect and decode FX.25.
3. Produce AX.25 frame bytes if FX.25 decoding succeeds.
4. Decode AX.25.
5. Create AX.25 connection events.
6. Process events through the connection table.

M1.35 starts at step 5 for decoded AX.25 frames. It does not implement FX.25
decode or FEC.

## TX Boundary

Future outbound flow should keep the same separation:

1. AX.25 state machine returns action intents.
2. Action mapper creates AX.25 frame plans.
3. Frame builder creates raw AX.25 frame bytes.
4. Optional future FX.25 wrapping occurs after AX.25 bytes exist.
5. KISS or another transport framing layer sends bytes if TX gates allow it.

M1.35 stops at stored frame plans. It does not build live queued frames,
dispatch frames, or wrap FX.25.

## Deferred Work

FX.25 tag detection, Reed-Solomon/FEC decode, FX.25 encode, KISS integration,
and live AX.25 connected-mode response queueing are deferred.
