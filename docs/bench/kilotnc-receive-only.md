# KiloTNC Receive-Only Bench

Topology:

```text
radio/audio/TNC input -> KiloTNC -> KISS serial or TCP -> KiloNode
```

Use KiloTNC in receive or monitor mode where applicable. KiloTNC-specific
details may evolve in M6VPN/KiloTNC, so this page only documents the KiloNode
side of the bench.

Serial KISS:

```text
packaging/examples/kilonode-rx-bench-serial-kiss.conf
```

TCP KISS:

```text
packaging/examples/kilonode-rx-bench-tcp-kiss.conf
```

Validation commands:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 connections
```

KiloTNC hardware is not required for automated tests.
