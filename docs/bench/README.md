# Receive-Only Bench Validation

This bench pack is for receive-only AX.25 validation. It feeds KISS data into
KiloNode, checks decoded RX state, and checks AX.25 live diagnostic counters.
It does not enable transmit.

Supported input paths:

- Dire Wolf TCP KISS from a USB sound card
- KiloTNC KISS, when available
- Serial KISS TNC
- TCP KISS
- PTY KISS
- Unix socket KISS

Primary validation commands:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --check-config
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --foreground
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 connections
./build/kilonodectl --socket /tmp/kilonode/control.sock rx events
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
```

Read [receive-only-safety.md](receive-only-safety.md) before connecting bench
hardware.
