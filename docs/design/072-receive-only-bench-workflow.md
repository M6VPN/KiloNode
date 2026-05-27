# Receive-Only Bench Workflow

M1.38 adds practical bench documentation and helper scripts for observing real
AX.25 receive traffic without enabling any transmit path.

## Scope

The bench workflow covers receive-only KISS inputs from:

- Dire Wolf TCP KISS fed by a USB sound card
- KiloTNC serial or TCP KISS
- serial KISS TNCs
- TCP KISS endpoints
- PTY KISS endpoints
- Unix socket KISS endpoints

The workflow validates decoded receive events, heard-list updates, and AX.25
live diagnostics. It does not create live connected sessions, enqueue response
frames, dispatch transmit frames, or deliver connected-mode payloads to shell or
BBS code.

## Standard Flow

1. Build KiloNode.
2. Check the chosen receive bench config with `kilonoded --check-config`.
3. Start the receive-only KISS source manually.
4. Start `kilonoded --foreground` with the matching bench config.
5. Confirm `kilonodectl tx gates` reports transmit blocked.
6. Check `kilonodectl rx status` and `kilonodectl heard`.
7. Check `kilonodectl ax25 live`.
8. Check `kilonodectl ax25 connections` only when connected-mode frames are
   expected.

The helper script `scripts/bench-rx-check-configs.sh` validates all bench config
files without hardware. `scripts/bench-rx-ax25-status.sh` queries the read-only
control commands when a daemon is already running.

## Expected Outcomes

UI frames should update receive events and heard-list state. They should not
create AX.25 connected diagnostics. SABM, SABME, DISC, UA, DM, RR, RNR, REJ,
and I frames may update the AX.25 diagnostics counters when the live RX feed is
explicitly enabled and the frames are relevant to the configured local callsign.

Generated frame plans remain diagnostic records only. They are not converted to
TX frames, queued, or dispatched.

## CI Boundary

Automated tests and checks validate configs, scripts, docs, and synthetic packet
fixtures only. They do not require a USB sound card, radio, TNC, Dire Wolf,
KiloTNC, serial device, or external service.

## Manual Hardware Use

Bench hardware is useful when validating real demodulated AX.25 input levels,
KISS transport behaviour, and live diagnostic counters. Manual validation should
use receive audio or monitor-mode data only, with transmit disabled before the
daemon is started.
