# Local TNC Transports

Serial, PTY, and Unix socket transports sit under the existing transport
abstraction. They move bytes only. KISS frame parsing, AX.25 decoding, and
monitor formatting happen above the transport layer.

All transports in this pass use blocking I/O. Clean EOF, hangup, or socket
disconnect ends monitor mode normally. Short reads and short writes are handled
by the common transport read/write helpers.

Serial transport opens a device path, applies raw termios mode, and supports a
fixed baud-rate set for common packet-radio TNC speeds. It does not change file
permissions or perform privileged setup.

PTY transport creates a master/slave pair and prints the slave path in monitor
mode. This is intended for local tests and integration with tools that can attach
to a TTY path.

Unix socket transport supports stream client mode and a single-client server
mode. Server mode unlinks only the exact configured socket path before binding.
Users should place sockets under a runtime directory such as `/tmp/kilonode/` or
`/run/user/...`.

Deferred work:

- Reconnect logic
- Multi-client services
- Permission management
- Daemon lifecycle
- Config file integration
- Routing, connected-mode AX.25, and BBS behavior

KiloTNC testing can use serial KISS mode when the device is exposed as a local
TTY. Dedicated compatibility tests remain future work.
