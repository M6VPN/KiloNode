# KISS Notes

This is a clean-room KISS implementation. It does not use LinBPQ, BPQ32, or
other GPL source code.

This pass implements an incremental byte-stream parser for KISS frames:

- `FEND` delimited frame boundaries
- Repeated and empty `FEND` handling
- `FESC TFEND` and `FESC TFESC` unescaping
- Invalid escape detection
- Oversized frame detection and recovery
- KISS type byte preservation
- Command nibble and port nibble decoding

Stream parsing is separate from AX.25 decoding. The stream parser only turns
hostile byte input into complete unescaped KISS frames. A KISS data frame uses
command nibble `0`; its payload can then be passed to AX.25 decoding.

The KISS type byte uses the low nibble for command and the high nibble for port.
Non-data command frames are parsed and reported, but they are not interpreted by
transport or modem control logic in this pass.

Not implemented yet:

- Serial, TCP, PTY, Unix socket, or daemon transports
- KiloTNC transport integration
- KISS command execution
- AX.25 connected-mode behavior
- NET/ROM routing
