# AX.25 Loopback Payload Delivery

M2.2 extends the in-memory loopback simulator with diagnostics-only payload
delivery records.

## Flow

The payload path is:

```text
event A send-i text="hello"
A builds raw AX.25 I frame
in-memory link gives bytes to B
B decodes I frame
B validates receive sequence
B records delivery diagnostics
B prepares RR acknowledgement
in-memory link gives RR to A
A updates acknowledgement state
```

The same flow supports bounded binary payloads through `hex=` script input.

M2.3 adds `segment=true` for payloads larger than paclen. The simulator sends
one I frame per segment, waits for the peer RR under window-size 1, and records
one reassembly diagnostic after all segments are accepted.

## Delivery Diagnostics

Each endpoint owns a bounded delivery queue. A record stores:

- delivery ID
- endpoint name
- port name
- source callsign
- destination callsign
- N(S)
- N(R)
- payload length
- bounded payload preview
- text or binary flag
- accepted or rejected status
- rejection reason

Delivery records are diagnostics only. They are not persisted and are not routed
to shell, BBS, RF command, NET/ROM, or forwarding code.

## Sequence Mismatch

If a receiver sees an unexpected N(S), the endpoint records a rejected delivery
diagnostic and the existing state machine produces a REJ action. The prepared
REJ frame remains in diagnostics and is moved only through the in-memory
loopback link.

## Limits

Payload size is bounded by AX.25 params. Loopback scripts reject malformed hex
payloads and overlong payload command fields. Segmented payloads are bounded by
the simulator payload limit and by the fixed segment count.
