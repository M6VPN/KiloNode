# Dire Wolf USB Sound Card TCP KISS

Topology:

```text
radio RX audio or test audio source -> USB sound card -> Dire Wolf -> TCP KISS -> KiloNode
```

Use Dire Wolf as a manually started receive-side decoder. Configure it to expose
TCP KISS on localhost, for example `127.0.0.1:8001`. Do not configure PTT for
this receive-only bench.

KiloNode config:

```text
packaging/examples/kilonode-rx-bench-tcp-kiss.conf
```

Validation flow:

1. Start Dire Wolf manually with TCP KISS enabled on localhost.
2. Check the KiloNode config:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --check-config
```

3. Start KiloNode:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --foreground
```

4. Check status:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 connections
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
```

Troubleshooting:

- No frames heard: check Dire Wolf audio input, KISS port, and KiloNode port.
- Audio too low or high: adjust input gain outside KiloNode.
- Wrong sound card: verify Dire Wolf is using the intended input.
- Wrong KISS port: confirm Dire Wolf and KiloNode both use localhost port 8001.
- UI frames only update RX/heard state and do not create AX.25 connected diagnostics.
- SABM, DISC, RR, RNR, REJ, UA, DM, or I frames are needed for connected diagnostics.
