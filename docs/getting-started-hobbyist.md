# Getting Started for Hobbyists

This guide is the short path for trying KiloNode without packet radio hardware.
It uses local tools, synthetic AX.25 loopback fixtures, and the local BBS store.

KiloNode can currently demonstrate receive-side tooling, local control, local
BBS storage, and in-memory AX.25 connected-mode behavior. It does not provide a
live RF CONNECT command or transmit AX.25 responses.

## Build

```sh
./scripts/build.sh
```

Run the hobbyist smoke check:

```sh
./scripts/hobbyist-smoke.sh
./scripts/hobbyist-first-run.sh
```

Expected output is a short list of `OK` lines ending with:

```text
OK hobbyist-smoke hardware=false daemon_started=false tx_writes=0 dispatch=0 fx25=0
```

## Local Config

Use the hobbyist local config for a localhost-only preview:

```sh
./build/kilonoded --config packaging/examples/kilonode-hobbyist-v0.2-alpha.conf --check-config
```

To run the daemon in the foreground:

```sh
./build/kilonoded --config packaging/examples/kilonode-hobbyist-v0.2-alpha.conf --foreground
```

In another terminal, query status:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock status
```

The example config keeps transmit disabled, dispatch disabled, connected-mode
AX.25 disabled, and the prepared-to-TX bridge disabled.

## AX.25 Loopback Demo

Run a local AX.25 I/RR payload loopback:

```sh
./build/kilonode-compat run-ax25-loopback tests/fixtures/ax25-loopback/connect-i-rr-disconnect.loop
```

This exchanges AX.25 bytes in memory only. It does not use KISS hardware, RF,
or the real TX queue.

Run a CONNECT dry-run:

```sh
./build/kilonode-compat run-ax25-connect-dry-run tests/fixtures/ax25-connect-dry-run/basic.connectdry
```

This validates a hypothetical local-admin CONNECT plan and reports that no
connection was created and no transmit path was used.

## Local BBS Storage

The hobbyist config enables the local BBS store and local TCP shell. It is a
local feature only. It is not RF BBS access and it is not connected-mode AX.25
payload delivery.

The default store path is:

```text
./var/messages
```

## Receive-Only Bench Work

Receive-only bench configs live in `packaging/examples/kilonode-rx-bench-*.conf`.
Use those when listening to a local KISS source. Keep `tx-enabled false` unless
a later release explicitly documents a real TX lab flow.

Dire Wolf TCP KISS is the recommended first receive-only modem path. VARA FM,
VARA HF, Mercury OFDM, and ARDOP are planned external modem adapter targets and
are not working KiloNode integrations yet.

Inspect modem profiles after starting a daemon with control enabled:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock modem-profiles
./build/kilonodectl --socket /tmp/kilonode/control.sock modems
```

Mercury OFDM is visible as a planned profile only. KiloNode does not launch
Mercury or assume its interface details yet.

For v0.2-alpha readiness, run:

```sh
./scripts/v02-alpha-readiness-check.sh
./scripts/mercury-discovery-check.sh
./scripts/mercury-status-placeholder.sh
```

Product preview notes live in
`docs/product/hobbyist-v0.2-alpha-preview.md`.

## Current Limits

- No live CONNECT command.
- No real TX queue writes.
- No TX dispatch.
- No RF BBS access.
- No NET/ROM routing.
- No BBS forwarding.
- No FX.25 FEC or wrapping.
