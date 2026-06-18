# AX.25 Windowed Loopback Diagnostics

M2.4 adds a simulator-only outstanding-frame diagnostic layer for AX.25
loopback I-frame tests.

Each loopback endpoint owns a bounded outstanding list. When the simulator
builds an I frame, it records:

- N(S)
- N(R)
- payload length
- segment index
- in-flight, acknowledged, or rejected status

RR frames from the peer clear in-flight records according to modulo-8 N(R).
The report records current outstanding count, maximum in-flight count,
acknowledged count, rejected count, and window-blocked attempts.

The window layer does not enqueue frames, dispatch frames, start timers, or
create a retransmission buffer. It only proves that the simulator can keep more
than one I frame in flight and then account for RR acknowledgements.

## Safety Boundary

This is not a live connected-mode transport. It remains inside
`kilonode-compat` loopback fixtures and unit tests.

The following remain blocked:

- live CONNECT
- shell or BBS payload binding
- real TX queue writes
- dispatch
- RF BBS
- NET/ROM
- FX.25 wrapping or FEC
