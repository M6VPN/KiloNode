# TCP KISS Receive-Only Bench

Topology:

```text
TCP KISS source on localhost -> KiloNode TCP KISS client
```

Config example:

```text
packaging/examples/kilonode-rx-bench-tcp-kiss.conf
```

The example connects to `127.0.0.1:8001`, matching common Dire Wolf TCP KISS
bench setups.

Validation:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --check-config
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
```

Common failures:

- KISS source not listening.
- Wrong localhost port.
- Firewall or namespace mismatch.
- Only UI traffic present, so AX.25 connected diagnostics stay empty.

Do not bind bench listeners to public addresses by default.
