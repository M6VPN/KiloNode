# Mercury OFDM Test Plan

The Mercury path should advance in stages:

| Stage | Scope | TX status |
| ----- | ----- | --------- |
| 0 | Status-only profile and documentation | blocked |
| 1 | Interface discovery with no socket connection | blocked |
| 2 | TCP status probe, no TX | blocked |
| 3 | Receive-only capture/import | blocked |
| 4 | Local loopback or simulator if Mercury has a test mode | blocked |
| 5 | Lab-only TX behind KiloNode safety gates | future |

Current status is Stage 0. Stage 2 and later require documented interface
details. Stage 5 requires a separate response TX safety review.
