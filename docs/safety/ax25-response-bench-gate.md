# AX.25 Response Bench Gate

AX.25 response TX validation is staged. KiloNode currently supports Stage 0
and receive-only Stage 1 work. Stage 2 and Stage 3 remain future and blocked.

| Stage | Name | Entry criteria | Exit criteria | Current status |
|-------|------|----------------|---------------|----------------|
| 0 | Offline replay only | synthetic fixtures and no hardware | prepared replay, timer replay, and no-transmit checks pass | current |
| 1 | Receive-only real source | RX config, TX blocked, no PTT | heard/RX/AX.25 diagnostics match captures | allowed manual |
| 2 | Dummy-load TX lab | checklist complete, explicit lab config, dummy load | controlled non-radiating TX evidence | future blocked |
| 3 | Controlled over-air legal test | Stage 2 passed, legal authority, operator preflight | reviewed logs and rollback plan | future blocked |

No current script enters Stage 2 or Stage 3. No current script opens hardware or
dispatches TX.
