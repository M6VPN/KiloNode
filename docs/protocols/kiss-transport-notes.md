# KISS Transport Notes

KISS frames can arrive over several byte-stream transports, including serial,
TCP, PTY, Unix sockets, and stdio. The transport only carries bytes. KISS frame
boundaries and escaping are handled by the KISS stream parser.

This pass implements:

- stdin/stdout KISS input for file and pipeline testing
- Minimal blocking TCP KISS client mode
- Minimal blocking TCP KISS server mode accepting one connection
- Serial KISS for hardware TNCs and KiloTNC-style devices
- PTY KISS for virtual TNC workflows
- Unix socket KISS for local integration scripts and future daemon work

Typical uses:

- Serial for hardware TNCs and KiloTNC
- PTY for virtual TNC workflows
- Unix socket for local tools and future daemon/control integration
- TCP for Dire Wolf-style TCP KISS workflows
- stdio for tests and pipelines

KiloTNC integration is partial until dedicated compatibility tests exist. Dire
Wolf-style TCP KISS compatibility is a target, but this pass does not claim full
compatibility.

Not implemented yet:

- Reconnect handling
- Multi-client TCP
- KISS command execution
