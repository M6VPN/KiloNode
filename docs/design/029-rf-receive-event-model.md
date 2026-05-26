# RF Receive Event Model

The receive event model records decoded inbound AX.25 frames from configured
KISS transports. It is diagnostic receive-side state only. It does not transmit
packets, route frames, or hand packets to BBS commands.

## Event Fields

Each event stores:

- runtime event ID
- timestamp
- configured port name
- KISS port and command
- source and destination callsigns
- digipeater path summary
- AX.25 control byte
- PID when present
- frame kind
- payload length
- bounded payload preview
- decode status
- malformed flag

Event IDs are monotonic within one daemon runtime and start at 1.

## Frame Classification

Frame kind is classified from the AX.25 control byte as UI, I, S, U, unknown,
or malformed. This is classification only. No connected-mode state transitions
are performed.

## Payload Preview

Only a bounded preview is stored. The default preview limit is 80 bytes.
Printable payloads are stored as escaped text. Binary payloads are stored as
hex. Full payloads are not retained in the receive event queue.

## Queue

The daemon owns a bounded ring buffer of recent events. The default maximum is
256 events. When full, the oldest event is evicted deterministically.

## Control Commands

Read-only control commands:

- `RX STATUS`
- `RX EVENTS`
- `RX EVENTS LIMIT <n>`
- `RX EVENTS PORT <name>`
- `RX EVENTS FROM <callsign>`
- `RX EVENTS TO <callsign>`
- `RX EVENT <id>`

All responses are bounded and multiline responses end with `END`.

This model is KiloNode-native and not compatibility behavior.
