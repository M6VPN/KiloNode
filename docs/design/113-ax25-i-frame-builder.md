# AX.25 I-Frame Builder

M2.2 adds raw AX.25 I-frame helpers for the offline loopback simulator.

The helpers cover modulo 8 only:

- build an I-frame control byte from N(S), N(R), and P/F
- decode N(S), N(R), and P/F through the existing control decoder
- reject sequence values outside 0 through 7
- keep modulo 128 planned

## Raw Frame Builder

The I-frame builder accepts:

- source callsign
- destination callsign
- optional digipeater path
- N(S)
- N(R)
- P/F bit
- PID
- payload pointer and length
- maximum information length
- caller-owned output buffer

The output is an AX.25 body:

```text
destination address
source address
optional digipeaters
control
pid
payload
```

It does not add HDLC flags, FCS, KISS command bytes, KISS escaping, FX.25
wrapping, or dispatch metadata.

Binary payloads are allowed. All payload handling uses explicit lengths and
does not depend on NUL termination.

## Decode Helper

The decode helper accepts raw AX.25 bytes that already sit at the AX.25 body
boundary. It validates that the control field is an I frame, extracts sequence
fields, exposes the payload pointer and length, and creates a bounded preview
for diagnostics.

The helper does not store unbounded payloads and does not deliver data to shell
or BBS code.
