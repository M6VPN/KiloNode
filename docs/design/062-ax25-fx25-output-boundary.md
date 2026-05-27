# AX.25 And FX.25 Output Boundary

AX.25 connected-mode action mapping produces raw AX.25 frame bytes. FX.25
remains a separate future layer.

## Current Output Flow

Current M1.34 flow:

1. AX.25 state core emits action intents.
2. AX.25 action mapper creates frame plans.
3. AX.25 frame builder emits raw AX.25 frame bytes.

No queueing or dispatch happens in this flow.

## Later KISS Flow

Later KISS TX work can wrap raw AX.25 bytes as KISS payloads, add the KISS
command byte, escape KISS special bytes, and write to an explicitly allowed
transport.

## Later FX.25 Flow

Future FX.25 TX work would wrap already-built AX.25 frame bytes before final
physical or TNC framing, when FX.25 is enabled and validated.

Future FX.25 RX work would decode or reject FX.25 first, then pass recovered
AX.25 frame bytes into normal AX.25 decode and connected-mode handling.

## Not Implemented

This pass does not add FX.25 FEC encode/decode, FX.25 correlation detection,
or any coupling between FX.25 and the AX.25 connection state machine.
