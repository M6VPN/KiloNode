# M1 Next Milestones

The M2 sequence should stay conservative and keep real TX behind explicit safety
gates.

| Milestone | Direction | Notes |
|-----------|-----------|-------|
| M2.1 | AX.25 connected-mode loopback simulator | Complete, test-only memory loopback. |
| M2.2 | AX.25 I-frame builder and loopback payload delivery | Complete, diagnostics-only payload delivery. |
| M2.3 | AX.25 segmentation, paclen, and loopback payload limits | Complete, window-size 1 baseline. |
| M2.4 | AX.25 windowed loopback and outstanding-frame diagnostics | Complete, still simulator-only. |
| M2.5 | Retransmission buffer diagnostics and retry replay polish | Complete, no dispatch or real TX. |
| M2.6 | Controlled local-admin CONNECT dry-run planning | Active, offline planning without RF or real TX. |
| M2.7 | Connected-mode shell over loopback | No RF exposure until loopback behaviour is proven. |
| M2.8 | RF BBS access planning | Separate auth, sysop, abuse, and safety design. |
| M2.9 | LinBPQ black-box observation import | Manual observations only, no source inspection. |
| M2.10 | FX.25 detection planning and test vectors | Detection and FEC vectors before decode or wrap claims. |

Do not combine real TX, CONNECT, RF BBS, NET/ROM, forwarding, and FX.25 into one
milestone. Each needs its own safety and compatibility proof.
