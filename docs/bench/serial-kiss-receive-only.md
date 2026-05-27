# Serial KISS Receive-Only Bench

Topology:

```text
serial KISS TNC -> KiloNode serial KISS port
```

Config example:

```text
packaging/examples/kilonode-rx-bench-serial-kiss.conf
```

Edit the serial device path before use. The example keeps the serial port
disabled by default so config checks do not open hardware.

Validation:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-serial-kiss.conf --check-config
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
```

Common failures:

- Wrong device path.
- Wrong baud rate.
- Hardware flow-control mismatch.
- TNC not in KISS mode.
- TX accidentally enabled on the TNC.

Keep KiloNode transmit disabled.
