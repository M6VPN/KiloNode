# AX.25 Diagnostic Replay Harness

M1.40 adds an offline replay harness for receive-only bench captures. The
harness is test-only and does not open live transports.

## Architecture

The replay path is:

```text
bench capture parser
KISS or raw AX.25 frame extraction
AX.25 decode
AX.25 RX diagnostics feed
AX.25 runtime counters and connection table
deterministic report
```

KISS captures are decoded at the packet boundary. Raw AX.25 captures use the
existing AXIP/AXUDP capture mode as a packet-boundary carrier for AX.25 bytes.

## Runtime Defaults

Replay uses a deterministic AX.25 runtime:

- AX.25 enabled
- connected-mode runtime dispatch disabled
- diagnostics enabled
- live RX feed enabled
- live RX diagnostic connection creation enabled
- frame-plan retention enabled
- local callsign `M6VPN-1`
- bounded connection table

These defaults exist only inside the offline harness. They do not enable live
CONNECT or daemon TX behaviour.

## Supported Frames

UI frames are decoded and counted as ignored by connected-mode diagnostics.
Connected-mode diagnostic events are created for supported U, S, and I frame
controls. Unsupported or malformed captures fail deterministically unless they
are marked as planned placeholders.

## FX.25 Boundary

FX.25 remains outside this harness. Future FX.25 decode must unwrap to AX.25
bytes before the replay feed, and future FX.25 encode must happen after AX.25
response frame generation. No FEC encode or decode is implemented here.

## Safety

The replay harness never writes to a TX queue, dispatches frames, opens serial
ports, opens sockets, starts Dire Wolf, starts KiloTNC, runs LinBPQ, or delivers
I-frame payloads to shell or BBS code.
