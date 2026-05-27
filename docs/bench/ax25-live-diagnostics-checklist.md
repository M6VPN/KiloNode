# AX.25 Live Diagnostics Checklist

1. Build KiloNode:

```sh
./scripts/build.sh
```

2. Check the bench config:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --check-config
```

3. Start the daemon:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --foreground
```

4. Confirm TX is blocked:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
```

5. Confirm RX transport state:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock ports
./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
```

6. Feed a known AX.25 receive source.

7. Confirm RX events increase:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock rx events
```

8. Confirm heard entries update:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock heard
```

9. Confirm live AX.25 counters increase:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
```

10. Confirm AX.25 connections appear only for connected-mode frames:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 connections
```

11. Collect logs and command output if needed.

12. Stop the daemon cleanly.
