# Architecture

KiloNode is built in layers so packet framing, transports, node behavior, and
BBS behavior can be tested independently.

| Layer | Scope | Status |
| ----- | ----- | ------ |
| Core library | Buffers, logging, config, callsign parsing | partial |
| Protocols | AX.25 and KISS encode/decode | partial |
| Transports | Serial, TCP, Unix socket, PTY, stdio, KiloTNC | planned |
| Node | Commands, ports, routes, heard list, connect handling | planned |
| BBS | Users, mail areas, message store, forwarding queue | planned |
| Compatibility | BPQ-style config migration and conformance tests | planned |
| Hardening | Fuzzing, hostile input tests, privilege separation | planned |

The first implementation target is `libpacketcore`, containing memory-safe
buffer primitives, callsign and SSID parsing, AX.25 frame work, and KISS frame
work. No external libraries are used in the scaffold.
