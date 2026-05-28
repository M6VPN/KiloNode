# M1 Next Milestones

The M2 sequence should stay conservative and keep real TX behind explicit safety
gates.

| Milestone | Direction | Notes |
|-----------|-----------|-------|
| M2.1 | Live AX.25 diagnostic scheduler polish | Improve diagnostics and reset/report behaviour without TX. |
| M2.2 | Prepared-to-TX bridge in test-only mock mode | Exercise conversion against mock queues only. |
| M2.3 | Controlled local-admin CONNECT dry-run | Local admin-only command planning without RF or real TX. |
| M2.4 | Connected-mode RX/TX loopback with memory transport | Test-only loopback before any real KISS TX. |
| M2.5 | Real KISS TX bridge in lab-only mode | Requires dummy-load or isolated lab policy and safety checklist. |
| M2.6 | Connected-mode shell over loopback | No RF exposure until loopback behaviour is proven. |
| M2.7 | RF BBS access planning | Separate auth, sysop, abuse, and safety design. |
| M2.8 | LinBPQ black-box observation import | Manual observations only, no source inspection. |
| M2.9 | FX.25 detection planning and test vectors | Detection and FEC vectors before decode or wrap claims. |

Do not combine real TX, CONNECT, RF BBS, NET/ROM, forwarding, and FX.25 into one
milestone. Each needs its own safety and compatibility proof.
