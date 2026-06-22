# Hobbyist v0.2-alpha Preview

KiloNode v0.2-alpha is intended to feel useful before live transmit features are
enabled. It focuses on local operation, receive-only packet monitoring,
compatibility fixtures, AX.25 loopback demos, local BBS storage, and external
modem status visibility.

## What can I try today?

You can build KiloNode, validate safe example configs, run no-hardware smoke
checks, inspect the local control plane, try the local BBS store, replay AX.25
fixtures, and run in-memory AX.25 loopback demos.

```sh
./scripts/build.sh
./scripts/hobbyist-smoke.sh
./scripts/hobbyist-first-run.sh
./scripts/hobbyist-status.sh
./scripts/v02-alpha-readiness-check.sh
```

## What do I need?

A POSIX shell, a C17 compiler, CMake, and the normal local build tools already
used by this project. No radio, TNC, USB sound card, Mercury, Dire Wolf, VARA,
ARDOP, or KiloTNC hardware is required for the default checks.

## What works without hardware?

The local daemon config checks, control socket commands, local BBS storage,
bench fixture replay, AX.25 timer replay, prepared-frame diagnostics, AX.25
loopback, I-frame delivery, segmentation diagnostics, and no-transmit checks
all run without hardware.

## What works with receive-only KISS?

Dire Wolf TCP KISS is the first practical hobbyist receive path. KiloNode can
monitor KISS frames from a local TCP KISS port with transmit disabled. KiloTNC
KISS receive is documented as a manual hardware path.

## What does not work yet?

Live CONNECT, live RF connected-mode sessions, real TX queue writes, TX
dispatch, RF BBS access, NET/ROM, forwarding, LinBPQ command compatibility,
Mercury implementation, VARA and ARDOP adapters, and FX.25 FEC remain blocked
or planned.

## How do I confirm TX is blocked?

Run:

```sh
./scripts/ax25-no-transmit-check.sh
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
```

The first command checks repository configs. The second command queries a
running local daemon.

## How do I view modem status?

With a daemon running:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock modem-profiles
./build/kilonodectl --socket /tmp/kilonode/control.sock modem-profile mercury-ofdm
./build/kilonodectl --socket /tmp/kilonode/control.sock modems
```

Mercury OFDM appears as a planned external modem profile. KiloNode does not
launch Mercury or assume its interface in this pass.

## How do I try the local BBS?

Use the hobbyist config and local shell from
`packaging/examples/kilonode-hobbyist-v0.2-alpha.conf`. The BBS path is local
storage only. RF BBS access and forwarding are not enabled.

## How do I run AX.25 loopback demos?

```sh
./scripts/ax25-loopback-fixtures.sh
./build/kilonode-compat run-ax25-loopback-dir tests/fixtures/ax25-loopback
```

These demos exchange frames in memory only. They do not touch a real TX queue.
