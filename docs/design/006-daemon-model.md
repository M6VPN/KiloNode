# Daemon Model

`kilonoded` is a foreground monitor daemon in this pass. It reads a KiloNode
native config file, opens enabled KISS ports, feeds raw bytes into the KISS
stream parser, and prints AX.25 monitor lines.

The runtime flow is:

```text
transport bytes -> KISS stream parser -> AX.25 decode -> monitor line
```

Transports do not decode frames. The daemon owns one KISS parser per enabled
port so partial frames survive arbitrary read boundaries.

This pass uses `poll` over transport file descriptors. TCP and Unix socket
listen transports accept one client when their listening descriptor becomes
readable. Clean EOF or disconnect closes that port. SIGINT and SIGTERM stop the
loop, close transports, free config state, and exit cleanly.

Logging is intentionally simple:

- stderr for daemon status and errors
- stdout for monitor lines

Deferred work:

- Background daemon mode
- syslog
- reconnect logic
- multi-client services
- privilege separation
- config reload
- NET/ROM routing
- connected-mode AX.25
- BBS and forwarding
