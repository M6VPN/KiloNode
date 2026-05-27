# AX.25 Connection Table

M1.35 adds a bounded AX.25 connection table scaffold for decoded AX.25
connected-mode events. It is an internal unit-tested layer only. It is not
connected to daemon RX/TX, local shell commands, RF UI commands, BBS sessions,
or any dispatch path.

## Purpose

The table owns connection records keyed by configured port, local callsign,
remote callsign, and optional digipeater path. Each record contains an AX.25
connection state object, a parameter snapshot, timestamps, counters, the last
state-machine actions, and the last generated frame plans.

## Key Fields

Keys contain:

| Field | Notes |
|-------|-------|
| Port name | Bounded configured port identifier. |
| Local callsign | Normalized callsign and SSID. |
| Remote callsign | Normalized callsign and SSID. |
| Digipeater path | Optional bounded list of normalized callsigns. |

The key layer validates callsigns and port text, rejects control characters,
rejects unsafe port characters, and formats keys deterministically.

## Record Fields

Each connection record stores:

| Field | Notes |
|-------|-------|
| Key | Stable table identity. |
| Connection state | Existing AX.25 state-machine object. |
| Parameter snapshot | Connected-mode parameters used for this record. |
| Created timestamp | Injected by caller for tests and future runtime use. |
| Last event timestamp | Updated on each processed event. |
| Last RX control | Decoded control summary from the event. |
| Last actions | State-machine action intents. |
| Last frame plans | Action-to-frame mapper output. |
| Counters | RX setup, teardown, I-frame, S-frame, and protocol-error counts. |

## Capacity Policy

The scaffold uses a fixed in-memory table with a default maximum of 32 records.
There is no unbounded allocation and no persistence. If the table is full, new
connections are rejected deterministically.

## Runtime Boundary

Processing an event calls the state machine and action-to-frame mapper, then
retains frame plans for diagnostics. It does not enqueue frame plans, dispatch
frames, start timers, deliver payloads, or expose `CONNECT`.
