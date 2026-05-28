# AX.25 Loopback Fixtures

These fixtures drive the M2.1 in-memory AX.25 loopback simulator. They exchange
raw AX.25 bytes between two simulator endpoints only.

The fixtures do not open KISS transports, do not use RF hardware, do not write
to the real TX queue, do not dispatch frames, and do not generate FX.25 frames.
