# KiloNode

[![CI](https://github.com/M6VPN/KiloNode/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/M6VPN/KiloNode/actions/workflows/ci.yml)

KiloNode is an independent ISC-licensed packet-radio node and BBS project. It
is being built as a clean-room replacement for LinBPQ-style packet nodes and
common TNC interfaces.

Current work covers AX.25 frame primitives, KISS stream parsing, local KISS
transports, a daemon skeleton, a local node shell, a KiloNode-native BBS store,
and local BBS/control queries. KiloTNC support is a first-class goal.

No LinBPQ or BPQ32 GPL code is used.

## Table of Contents

- [Requirements](#requirements)
- [Setup](#setup)
- [Usage](#usage)
- [Try KiloNode in 5 Minutes](#try-kilonode-in-5-minutes)
- [Build Profiles](#build-profiles)
- [Install](#install)
- [Receive-Only Bench Validation](#receive-only-bench-validation)
- [Supported and Planned Modems](#supported-and-planned-modems)
- [AX.25 Response Safety Gate](#ax25-response-safety-gate)
- [v0.1-alpha Readiness](#v01-alpha-readiness)
- [M2 Loopback Work](#m2-loopback-work)
- [Compatibility](#compatibility)
- [License](#license)

## Requirements

- [CMake](https://cmake.org/)
- C17 compiler, such as [Clang](https://clang.llvm.org/) or [GCC](https://gcc.gnu.org/)

No external libraries are required.

## Setup

```sh
./scripts/build.sh
```

Run tests:

```sh
./scripts/test.sh
```

## Usage

```sh
./build/kilonode
```

Check the example daemon config:

```sh
./build/kilonoded --config docs/examples/kilonode.conf --check-config
```

Run the local daemon in foreground:

```sh
./build/kilonoded --config docs/examples/kilonode.conf --foreground
```

Query the control socket:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock status
```

## Try KiloNode in 5 Minutes

The fastest no-hardware path is the hobbyist smoke check:

```sh
./scripts/build.sh
./scripts/hobbyist-smoke.sh
./scripts/hobbyist-first-run.sh
```

This checks the local hobbyist config, AX.25 loopback demo, CONNECT dry-run
planner, local BBS store test, and no-transmit gates.

The recommended local preview config is:

```sh
./build/kilonoded --config packaging/examples/kilonode-hobbyist-v0.2-alpha.conf --check-config
```

Run the v0.2-alpha readiness checks before using the hobbyist preview:

```sh
./scripts/v02-alpha-readiness-check.sh
```

Run a local AX.25 loopback payload demo:

```sh
./build/kilonode-compat run-ax25-loopback tests/fixtures/ax25-loopback/connect-i-rr-disconnect.loop
```

Run a local CONNECT dry-run:

```sh
./build/kilonode-compat run-ax25-connect-dry-run tests/fixtures/ax25-connect-dry-run/basic.connectdry
```

More detail is in
[Getting Started for Hobbyists](docs/getting-started-hobbyist.md) and
[Hobbyist v0.2-alpha Preview](docs/product/hobbyist-v0.2-alpha-preview.md).

## Build Profiles

Sanitizer build and tests:

```sh
./scripts/build-sanitize.sh
```

Release build with hardening:

```sh
./scripts/build-release.sh
```

Strict warning build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DKILONODE_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Project checks:

```sh
./scripts/check-headers.sh
./scripts/check-portability.sh
./scripts/check-format.sh
```

## Install

Install into a local prefix:

```sh
./scripts/install-local.sh --prefix /tmp/kilonode-install
```

Or install a built tree directly:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DKILONODE_HARDENING=ON
cmake --build build-release
cmake --install build-release --prefix /tmp/kilonode-install
```

Dry-run removal of installed files:

```sh
./scripts/uninstall-local.sh --prefix /tmp/kilonode-install --dry-run
```

Packaging checks:

```sh
./scripts/check-packaging.sh
```

Recommended runtime paths:

- Linux: `/etc/kilonode`, `/run/kilonode`, `/var/lib/kilonode`
- OpenBSD: `/etc/kilonode`, `/var/run/kilonode`, `/var/kilonode`
- FreeBSD and NetBSD: `/usr/local/etc/kilonode`, `/var/run/kilonode`, `/var/db/kilonode`

Service examples live under `packaging/`. They are examples for packagers and
are not installed into system service directories automatically.

## Receive-Only Bench Validation

Receive-only bench docs live in [docs/bench](docs/bench/README.md). They cover
Dire Wolf with a USB sound card, KiloTNC, and serial, TCP, PTY, and Unix socket
KISS inputs. Bench configs keep transmit disabled.
Synthetic bench capture fixtures live in
[tests/fixtures/bench](tests/fixtures/bench/README.md) and can be replayed
without hardware. AX.25 diagnostic replay feeds those fixtures into the
diagnostics runtime and checks that TX write attempts stay at zero.

```sh
./scripts/bench-rx-check-configs.sh
./scripts/bench-rx-replay-fixtures.sh
./scripts/bench-rx-replay-diagnostics.sh
./scripts/bench-rx-workspace-init.sh /tmp/kilonode-manual-captures
```

Manual capture workspaces are covered in
[manual-capture-workspace.md](docs/bench/manual-capture-workspace.md).

## Supported and Planned Modems

Current practical receive paths use KISS:

- Dire Wolf TCP KISS from a sound card.
- KiloTNC KISS, when available.
- Serial, TCP, PTY, and Unix socket KISS sources.

KiloNode also has a read-only external modem status scaffold:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock modem-profiles
./build/kilonodectl --socket /tmp/kilonode/control.sock modems
```

Planned external modem adapter targets:

- VARA FM.
- VARA HF.
- Mercury OFDM from Rhizomatica.
- ARDOP.

These planned modem adapters will be external process or TCP interface
boundaries. KiloNode will not vendor modem source code, and this scaffold does
not launch modem processes or enable transmit, PTT, live CONNECT, RF BBS, or
forwarding.

See [Modems](docs/modems/README.md) and
[M2.8 External Modem Scaffold](docs/milestones/M2.8-external-modem-scaffold.md).
Mercury-specific discovery material starts at
[Mercury OFDM Discovery Pack](docs/modems/mercury-ofdm-discovery-pack.md).

## AX.25 Response Safety Gate

AX.25 connected-mode response TX remains blocked. Prepared response frames are
diagnostics only, and the prepared-to-TX bridge stays disabled in runtime.

Safety gate docs live in [docs/safety](docs/safety/README.md). The current
non-transmitting checks are:

```sh
./scripts/ax25-no-transmit-check.sh
./scripts/ax25-safety-check.sh
./scripts/ax25-prepared-gate-report.sh
./scripts/ax25-response-bench-preflight.sh
```

Future real response TX requires the safety checklist, bench gate, operator
preflight, and a separate implementation milestone.

## v0.1-alpha Readiness

M1 v0.1-alpha is a receive/diagnostics readiness target. It includes KISS
monitoring, local daemon/control tools, local shell and BBS storage, AX.25
receive diagnostics, connected-mode diagnostic scaffolds, prepared-frame
diagnostics, compatibility fixtures, and no-transmit safety gates.

It is not a live connected-mode, RF BBS, NET/ROM, real TX, retransmission, or
FX.25 release. The readiness audit lives in
[docs/milestones/M1-v0.1-alpha-readiness.md](docs/milestones/M1-v0.1-alpha-readiness.md).

```sh
./scripts/m1-readiness-check.sh
./scripts/ax25-no-transmit-check.sh
./scripts/test.sh
```

## M2 Loopback Work

M2 is currently simulator-only. It adds in-memory AX.25 connected-mode loopback
flows, I-frame payload diagnostics, paclen segmentation, and windowed
outstanding-frame diagnostics. M2.7 adds the hobbyist preview path for trying
that work without hardware.

Current M2 checks:

```sh
./scripts/m2-readiness-check.sh
./scripts/hobbyist-smoke.sh
./scripts/ax25-loopback-fixtures.sh
./build/test_ax25_loopback_window
```

M2 still does not expose CONNECT, bind payloads to shell or BBS, write the real
TX queue, dispatch frames, or implement FX.25 wrapping.

## Compatibility

Compatibility tracking lives in:

- [LinBPQ compatibility matrix](docs/compat/linbpq-compatibility-matrix.md)
- [TNC interface matrix](docs/compat/tnc-interface-matrix.md)
- [Platform matrix](docs/compat/platform-matrix.md)

Status values are `planned`, `partial`, `implemented`, and `tested`.

## License

KiloNode is released under the ISC license.


## Mirrors:
