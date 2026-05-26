# KISS Transport Notes

KISS frames can arrive over several byte-stream transports, including serial,
TCP, PTY, Unix sockets, and stdio. The transport only carries bytes. KISS frame
boundaries and escaping are handled by the KISS stream parser.

This pass implements:

- stdin/stdout KISS input for file and pipeline testing
- Minimal blocking TCP KISS client mode
- Minimal blocking TCP KISS server mode accepting one connection

KiloTNC integration is a future target. Dire Wolf-style TCP KISS compatibility
is also a target, but this pass does not claim full compatibility.

Not implemented yet:

- Serial device setup
- PTY adapter
- Unix socket adapter
- Reconnect handling
- Multi-client TCP
- KISS command execution
