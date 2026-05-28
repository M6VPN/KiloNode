# M2 Plan

M2 extends the M1 receive and diagnostics base with controlled AX.25
connected-mode test surfaces. Real RF transmit, live CONNECT, RF BBS, NET/ROM,
and FX.25 remain blocked until separate safety gates are met.

| Milestone | Direction | Status |
|-----------|-----------|--------|
| M2.1 | AX.25 connected-mode loopback simulator | active |
| M2.2 | Prepared-to-TX bridge in stricter test-only mock mode | planned |
| M2.3 | Local-admin CONNECT dry-run planning | planned |
| M2.4 | Connected-mode loopback shell prototype | planned |
| M2.5 | Real KISS TX bridge in lab-only dummy-load mode | planned |
| M2.6 | Connected-mode shell over loopback | planned |
| M2.7 | RF BBS access planning | planned |
| M2.8 | LinBPQ black-box observation import | planned |
| M2.9 | FX.25 detection planning and test vectors | planned |

M2 work should keep each boundary narrow. Simulator and mock paths must prove
state, frame, timer, retry, and safety counters before any real TX milestone is
considered.
