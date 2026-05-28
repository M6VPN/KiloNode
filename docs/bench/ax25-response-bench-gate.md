# AX.25 Response Bench Gate

This bench gate mirrors the safety gate for future AX.25 response TX. Current
bench work remains receive-only or offline replay.

Stages:

| Stage | Scope | Current use |
|-------|-------|-------------|
| 0 | Offline replay of synthetic captures and timer scripts | supported |
| 1 | Receive-only real source through KISS | supported manually |
| 2 | Dummy-load or non-radiating TX lab | future blocked |
| 3 | Controlled legal over-air test | future blocked |

Stage 0 exits when `ax25-safety-check.sh` and `ax25-no-transmit-check.sh`
pass. Stage 1 exits when receive-only logs, heard entries, RX events, and AX.25
diagnostics match the captured source while TX gates remain blocked.

Stage 2 and Stage 3 require a separate milestone. No script in this repository
starts TX bench work.
