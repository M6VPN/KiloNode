# RF Transmit Frame Model

KiloNode has a transmit-side frame model for preparing outbound AX.25 frames
before any runtime dispatch exists. This pass is groundwork only. It does not
send packets, expose user transmit commands, or implement connected-mode AX.25.

## Frame Fields

Each transmit frame stores:

- runtime transmit ID
- created timestamp
- configured port name
- KISS port and command
- source and destination callsigns
- optional digipeater path
- AX.25 control byte
- optional PID byte
- frame kind
- payload length
- bounded payload preview
- raw AX.25 bytes
- raw KISS bytes
- status

Frame kinds are:

- `UI`
- `raw-ax25`
- `unknown`

Statuses are:

- `queued`
- `sent`
- `dropped`
- `failed`
- `dry-run`

The status names are diagnostic state only. They do not imply that dispatch to a
TNC is enabled.

## UI Builder

The UI builder accepts a source callsign, destination callsign, optional
digipeater path, PID, payload bytes, configured port name, and KISS port number.
It validates all callsigns, enforces payload and AX.25 frame limits, encodes the
AX.25 UI frame through the existing AX.25 primitives, and KISS-escapes the
result through the existing KISS helper.

The default PID for normal no-layer-3 text UI frames remains a caller choice.
Callers can pass `0xf0` when they want that public AX.25 convention.

## Raw AX.25 Builder

A controlled raw AX.25 builder exists for test and future internal use. It
accepts explicit bytes and length, enforces the AX.25 size limit, KISS-encodes
the frame, and marks the frame as `raw-ax25`.

It does not parse or rewrite raw bytes.

## KISS Boundary

Transmit frames store AX.25 bytes and a complete KISS frame. The KISS frame
starts and ends with `FEND` and contains the escaped KISS type byte plus AX.25
payload.

Runtime dispatch to configured transports is deferred.

## Payload Preview

Only a bounded payload preview is stored for diagnostics. Printable previews are
quoted and escaped. Binary previews are shown as hex. Full payload retention
outside the prepared frame bytes is intentionally avoided.

## Deferred Work

- actual dispatch to TNC transports
- shell transmit commands
- RF BBS replies
- AX.25 connected-mode state machine
- NET/ROM routing
- forwarding queues
- BPQ/LinBPQ interop tests
