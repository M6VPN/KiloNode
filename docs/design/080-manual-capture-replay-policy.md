# Manual Capture Replay Policy

Manual replay uses the M1.40 offline AX.25 diagnostics harness. It does not add
live protocol behaviour.

## Replay Policy

Replay accepts indexed `.capture` files from a manual workspace. Supported KISS
and raw AX.25 captures are decoded and passed into the AX.25 diagnostics
runtime. UI frames are counted as ignored by connected-mode diagnostics. SABM
and other supported connected-mode controls update diagnostic counters and table
state.

## Safety

Replay never:

- opens serial, TCP, PTY, or Unix transports
- starts Dire Wolf or KiloTNC
- runs LinBPQ
- queues TX frames
- dispatches TX frames
- exposes CONNECT
- delivers I-frame payloads to shell or BBS code

All reports include TX write counts. They must remain zero.

## FX.25

FX.25 manual captures may be imported and catalogued. They replay as unsupported
or planned until FX.25 detection, FEC, and AX.25 payload extraction exist.
