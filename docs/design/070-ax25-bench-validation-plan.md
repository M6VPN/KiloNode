# AX.25 Bench Validation Plan

Automated tests for M1.37 are synthetic and deterministic. They do not require
real TNC hardware, Dire Wolf, KiloTNC, a USB sound card, serial devices, or RF
equipment.

## Bench Instructions

Later manual validation can use a receive-only bench setup:

1. Attach a USB sound card, TNC, or other receive source to a known AX.25
   source.
2. Use Dire Wolf, KiloTNC, or another external tool to present packet-boundary
   KISS frames to KiloNode over TCP, serial, or PTY.
3. Keep transmit disabled in KiloNode and at the external modem or TNC.
4. Configure KiloNode with `ax25 enabled true` and `live-rx-feed true`.
5. Enable `live-rx-create-connections true` only when diagnostic connection
   creation from inbound SABM or SABME frames is desired.
6. Observe:

```text
kilonodectl --socket /tmp/kilonode/control.sock ax25 live
kilonodectl --socket /tmp/kilonode/control.sock ax25 connections
kilonodectl --socket /tmp/kilonode/control.sock ax25 counters
```

Do not connect a transmitter, enable TX dispatch, or enable RF replies while
validating receive diagnostics.

## Not Required In This Pass

No bench validation is required for M1.37. The implementation is covered by
synthetic unit tests and daemon scaffold tests.
