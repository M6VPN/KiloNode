# AX.25 Connection Event Flow

M1.35 adds a small event wrapper between decoded AX.25 frames and the isolated
connected-mode state machine. The wrapper carries enough metadata for table
lookup, state-machine input, and diagnostics without binding to daemon runtime
paths.

## Inbound Frame Mapping

Inbound events are created from decoded AX.25 frames, a configured port name,
and the configured local callsign. The event builder validates that the frame
is addressed to the local callsign before creating a connection key.

Supported inbound events include:

| Frame class | Event |
|-------------|-------|
| SABM/SABME | Setup request events. |
| UA/DM/DISC | Release and setup response events. |
| I | I-frame receive event with N(S), N(R), and payload length. |
| RR/RNR/REJ | Supervisory events with N(R). |

UI frames do not create connected-mode events. Malformed or non-local frames
are rejected before table processing.

## Local Test Events

The scaffold has helper constructors for local connect, local disconnect, T1
timeout, and T3 timeout events. These exist for unit tests and future internal
runtime work only. They are not exposed through the local shell, RF UI command
parser, BBS shell, or control socket.

## Table Processing

The table processing path is:

1. Validate event and connection key.
2. Find or create a record only for allowed setup events.
3. Convert the event into state-machine input.
4. Run the AX.25 state-machine core.
5. Map action intents into frame plans.
6. Store actions, frame plans, counters, and timestamps for diagnostics.

The output remains diagnostic data. No TX queue, transport, timer scheduler, or
payload delivery callback is called.

## Deferred Work

Live daemon session binding, real timer scheduling, I-frame payload delivery,
control-plane diagnostics, and RF/BBS shell attachment are deferred.
