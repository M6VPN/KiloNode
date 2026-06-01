# AX.25 Reassembly Diagnostics

M2.3 adds diagnostics-only reassembly records for segmented AX.25 loopback
payloads.

Each loopback endpoint owns a bounded reassembly queue. A record stores:

- reassembly ID
- endpoint name
- port name
- source callsign
- destination callsign
- expected segment count
- received segment count
- total payload length
- bounded payload preview
- text or binary flag
- complete status
- reason

Reassembly records are created by the simulator after all generated segments
arrive in order and are acknowledged under the current window-size 1 policy.

## Boundary

Reassembled payloads are diagnostics only. They are not persisted, delivered to
the node shell, delivered to the BBS, routed through NET/ROM, forwarded, or
promoted to RF transmit paths.

## Bounds

The queue is fixed-size. Payload preview length is fixed and shorter than the
maximum simulator payload. Reports show deterministic counts and bounded hex
previews for binary payloads.

Out-of-order segment modelling remains planned. Sequence mismatch diagnostics
continue to use the M2.2 rejected delivery path.
