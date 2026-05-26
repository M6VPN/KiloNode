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
- [Build Profiles](#build-profiles)
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

## Compatibility

Compatibility tracking lives in:

- [LinBPQ compatibility matrix](docs/compat/linbpq-compatibility-matrix.md)
- [TNC interface matrix](docs/compat/tnc-interface-matrix.md)
- [Platform matrix](docs/compat/platform-matrix.md)

Status values are `planned`, `partial`, `implemented`, and `tested`.

## License

KiloNode is released under the ISC license.


## Mirrors:
