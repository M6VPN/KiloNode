# Mercury OFDM Boundary

Mercury OFDM by Rhizomatica is a planned external HF OFDM modem target for
KiloNode. In this milestone KiloNode records Mercury as a first-class planned
profile only. It does not implement Mercury modem logic, DSP, framing, FEC,
process management, or TCP protocol handling.

KiloNode should talk only to a documented Mercury process or TCP interface when
that interface has been captured in project-owned notes. No Mercury source is
vendored, copied, linked, embedded, or used as an implementation reference.

## Discovery Checklist

- Mercury binary or project location.
- Licence and redistribution limits.
- Host/control interface.
- Data interface.
- Framing and message boundaries.
- Status commands.
- Receive path.
- Transmit path.
- PTT or radio-control responsibility.
- Sample receive capture.
- Non-transmitting or dummy test mode.

Until this checklist is complete, Mercury support remains planned and
status-only in KiloNode.
