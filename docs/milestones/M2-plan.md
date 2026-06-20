# M2 Plan

M2 extends the M1 receive and diagnostics base with controlled AX.25
connected-mode test surfaces. Real RF transmit, live CONNECT, RF BBS, NET/ROM,
and FX.25 remain blocked until separate safety gates are met.

| Milestone | Direction | Status |
|-----------|-----------|--------|
| M2.1 | AX.25 connected-mode loopback simulator | complete |
| M2.2 | AX.25 I-frame builder and loopback payload delivery | complete |
| M2.3 | AX.25 segmentation, paclen, and loopback payload limits | complete |
| M2.4 | AX.25 windowed loopback and outstanding-frame diagnostics | complete |
| M2.5 | Retransmission buffer diagnostics and retry replay polish | complete |
| M2.6 | Controlled local-admin CONNECT dry-run planning | active |
| M2.7 | Connected-mode shell over loopback | planned |
| M2.8 | RF BBS access planning | planned |
| M2.9 | LinBPQ black-box observation import | planned |
| M2.10 | FX.25 detection planning and test vectors | planned |

M2 work should keep each boundary narrow. Simulator and mock paths must prove
state, frame, timer, retry, and safety counters before any real TX milestone is
considered.
