# AX.25 Loopback Fixtures

These fixtures drive the M2.1 in-memory AX.25 loopback simulator. They exchange
raw AX.25 bytes between two simulator endpoints only.

M2.2 fixtures add text, binary, and sequence-mismatch I-frame payload cases.
Payload delivery is recorded as diagnostics only.

M2.3 fixtures add paclen-bounded segmented text and binary payload cases.
Reassembly is recorded as diagnostics only.

M2.4 fixtures add windowed segmented text and binary payload cases.
Outstanding I-frame state and RR acknowledgements are recorded as diagnostics
only.

The fixtures do not open KISS transports, do not use RF hardware, do not write
to the real TX queue, do not dispatch frames, and do not generate FX.25 frames.
