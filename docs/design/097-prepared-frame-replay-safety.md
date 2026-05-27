# Prepared Frame Replay Safety

Prepared replay assertions are offline checks. They do not open radios, TNCs,
serial ports, TCP KISS ports, PTYs, Unix sockets, Dire Wolf, KiloTNC, or
LinBPQ.

The checked object is the AX.25 prepared diagnostics queue. It stores frame
plans and optional raw AX.25 bytes for inspection only. It is separate from the
real TX queue and has no dispatch path.

Safety rules for this milestone:

- no CONNECT command
- no shell or BBS transmit command
- no RF command ingress for CONNECT
- no real TX queue writes
- no TX dispatch
- no retransmission dispatch
- no payload delivery to shell or BBS code
- no FX.25 FEC or wrapping

Every prepared replay command reports or checks TX writes as zero. A non-zero
TX write count is a failure.
