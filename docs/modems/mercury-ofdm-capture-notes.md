# Mercury OFDM Capture Notes

Future Mercury captures should use only externally visible interface traffic.
Do not copy or inspect Mercury source code for adapter behavior.

Capture notes should record:

- Mercury version or commit identifier.
- Capture date.
- Host OS and environment.
- Mercury command line or config used.
- KiloNode config used.
- Whether traffic is status, RX, test loopback, or TX lab.
- Whether packet boundaries are visible.
- Error messages from the external interface.

Store raw captures outside the repository until reviewed. Import only sanitized
fixtures through the manual capture workspace.
