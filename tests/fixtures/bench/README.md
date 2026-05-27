# KiloNode Bench Capture Fixtures

These fixtures are synthetic receive-only captures for testing KiloNode bench
tooling. They are not LinBPQ captures, are not sourced from GPL code, and do not
require radio hardware, Dire Wolf, KiloTNC, USB sound cards, serial devices, or
transmit capability.

The KISS fixtures include KISS framing. The raw AX.25 fixtures use the existing
AXIP capture method because the current packet capture format models AXIP and
AXUDP frames as packet-boundary AX.25 bytes.

`fx25-future-placeholder.capture` is a planned placeholder only. It exists so
bench coverage can track future FX.25 receive fixtures without claiming FX.25
decode or FEC support.

`ax25-diag-replay.expected` contains offline AX.25 diagnostic replay
expectations. The sequence capture files are planned placeholders until the
capture format supports multi-frame fixture bodies.
