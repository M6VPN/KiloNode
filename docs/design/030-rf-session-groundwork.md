# RF Session Groundwork

The observed RF session table tracks source and destination pairs seen in
decoded receive events. It is observation state, not connected-mode AX.25.

## Matching

Entries are keyed by:

- configured port name
- source callsign
- destination callsign

The same callsign pair on a different port creates a separate entry.

## Counters

Each entry stores first-seen and last-seen timestamps, total frames, UI frames,
I frames, S frames, U frames, malformed count, last control byte, last PID when
present, and last event ID.

## Limits

The default maximum is 128 observed sessions. When full, the entry with the
oldest last-seen timestamp is evicted.

## Control Commands

Read-only control commands:

- `RX SESSIONS`
- `RX SESSIONS PORT <name>`
- `RX SESSIONS FROM <callsign>`

## Future Use

This table is groundwork for later RF node shell access, RF BBS access,
connected-mode AX.25, and NET/ROM compatibility.

Deferred work includes transmit path, AX.25 state machines, RF authentication,
NET/ROM routing, BBS forwarding, and external interop tests.
