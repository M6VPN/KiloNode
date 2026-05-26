# AX.25 Notes

This is a clean-room AX.25 implementation. It does not use LinBPQ, BPQ32, or
other GPL source code.

This pass implements only frame primitives:

- Callsign and SSID address representation
- AX.25 address field encode and decode
- Destination, source, and digipeater address lists
- Digipeater repeated bit preservation
- Generic control byte decode
- PID decode when the control field indicates one
- UI frame encode and decode tests

KISS transports usually carry the AX.25 frame body without HDLC flags or FCS.
For that reason, this layer does not add, verify, or strip HDLC flags or FCS.

At a high level, each AX.25 address field entry is 7 bytes. The first 6 bytes
hold the callsign characters shifted left by one bit and padded with shifted
spaces. The seventh byte carries SSID bits, reserved protocol bits, a repeated
or command bit, and the final address bit. The final bit marks the last address
entry in the destination, source, and digipeater list.

Not implemented yet:

- Connected-mode AX.25 state machines
- NET/ROM routing
- Monitor text rendering
- FCS handling
- Segmentation, timers, retries, or link sessions
- BBS behavior
