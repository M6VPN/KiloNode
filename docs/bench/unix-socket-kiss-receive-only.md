# Unix Socket KISS Receive-Only Bench

Topology:

```text
local Unix socket KISS source -> KiloNode Unix socket KISS listener
```

Config example:

```text
packaging/examples/kilonode-rx-bench-unix-kiss.conf
```

The example listens at `/tmp/kilonode/rx-bench-kiss.sock`.

Validation:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-unix-kiss.conf --check-config
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
```

Common failures:

- Socket directory missing.
- Stale socket path from a previous run.
- Client sends bytes before KiloNode is listening.
- KISS framing errors.

Keep socket permissions local to the bench user.
