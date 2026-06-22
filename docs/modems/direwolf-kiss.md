# Dire Wolf TCP KISS

Dire Wolf TCP KISS is the current practical external modem path for hobbyist
receive-only testing. KiloNode can connect to a local TCP KISS endpoint and
decode received KISS/AX.25 frames into monitor, heard, RX, and AX.25 diagnostic
state.

Use `packaging/examples/kilonode-rx-bench-tcp-kiss.conf` as the receive-only
starting point. Keep `transmit.enabled false`, `dispatch-enabled false`, and
`tx-enabled false` on the KISS port.

The external modem profile name for this path is `kiss-tcp`. It is listed as a
receive-only working path because the existing KISS receive path is already in
KiloNode.
