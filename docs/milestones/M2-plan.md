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
| M2.6 | Controlled local-admin CONNECT dry-run planning | complete |
| M2.7 | Hobbyist preview release polish | complete |
| M2.8 | External modem support roadmap and scaffold | complete |
| M2.9 | Hobbyist v0.2-alpha readiness and Mercury discovery pack | active |
| M2.10 | RF BBS access planning | planned |
| M2.11 | LinBPQ black-box observation import | planned |
| M2.12 | FX.25 detection planning and test vectors | planned |

M2 work should now favor a usable hobbyist preview and practical modem paths
over deeper simulator-only detail. Dire Wolf receive-only use is the first
working hobbyist path. VARA FM/HF, Mercury OFDM, and ARDOP are future external
modem adapter targets. M2.9 adds product readiness checks and Mercury discovery
material without implementing a modem protocol. Real TX, live CONNECT, RF BBS,
NET/ROM, and FX.25 still require separate milestones.
