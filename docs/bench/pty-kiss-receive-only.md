# PTY KISS Receive-Only Bench

Topology:

```text
local KISS test source -> PTY slave -> KiloNode PTY KISS port
```

Config example:

```text
packaging/examples/kilonode-rx-bench-pty-kiss.conf
```

The PTY transport is useful for local bench tools that can write KISS bytes to
a pseudo-terminal. It does not require RF hardware.

Validation:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-pty-kiss.conf --check-config
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
```

Common failures:

- Test source opened the wrong PTY path.
- Writer sends raw AX.25 instead of KISS frames.
- No connected-mode frames are present.

Keep transmit disabled.
