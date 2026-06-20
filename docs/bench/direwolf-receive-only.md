# Dire Wolf Receive-Only Path

Dire Wolf is the recommended first hobbyist receive path for KiloNode because it
can expose decoded packets as TCP KISS.

Use the existing TCP KISS bench config:

```text
packaging/examples/kilonode-rx-bench-tcp-kiss.conf
```

Typical topology:

```text
radio RX audio or test audio -> sound card -> Dire Wolf -> TCP KISS -> KiloNode
```

Rules for this path:

- Start Dire Wolf manually.
- Configure KISS TCP on localhost.
- Do not configure PTT.
- Keep KiloNode `transmit enabled false`.
- Keep the KiloNode port `tx-enabled false`.

Validate KiloNode config:

```sh
./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --check-config
```

Then follow the detailed page:

```text
docs/bench/direwolf-usb-soundcard-kiss.md
```
