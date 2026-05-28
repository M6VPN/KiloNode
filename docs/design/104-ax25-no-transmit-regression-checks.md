# AX.25 No-Transmit Regression Checks

The no-transmit regression checks are shell-only and deterministic.

They check:

- safety docs and fixtures exist
- receive-only configs keep transmit disabled
- prepared bridge real mode is not enabled
- default examples do not enable real KISS dispatch
- prepared replay and timer replay report zero TX writes when tools are built
- scripts do not open devices or start external daemons

The checks do not start KiloNode, Dire Wolf, KiloTNC, serial devices, TCP KISS
services, PTYs, Unix sockets, or LinBPQ.
