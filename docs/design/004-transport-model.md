# Transport Model

Transports are byte-stream links. They read and write raw bytes only. They do
not decode KISS frames, AX.25 frames, node commands, or BBS data.

The live monitor flow is:

```text
transport bytes -> KISS stream parser -> monitor formatter
```

Read buffers and write buffers are caller-owned. Transport reads fill a
caller-provided buffer and return the number of bytes read. Transport writes
send bytes from a caller-provided buffer and handle short writes internally.

This pass uses simple blocking I/O. EOF and clean connection close are normal
end states. Input bytes remain hostile and are validated later by the KISS
stream parser and AX.25 decoder.

Implemented transports:

- stdin/stdout
- TCP client
- TCP server that accepts one connection

Deferred transports and features:

- Serial KISS
- PTY KISS
- Unix socket KISS
- Reconnect logic
- Multi-client TCP service
- TLS or authentication
- Daemon lifecycle
