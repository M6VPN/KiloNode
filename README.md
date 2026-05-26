# KiloNode

KiloNode is an independent ISC-licensed packet-radio node and BBS project. It
is being built as a clean-room replacement for LinBPQ-style packet nodes and
common TNC interfaces.

The first compatibility goals are AX.25 framing, KISS transports, and node/BBS
behavior that can interoperate with common packet-radio workflows. KiloTNC
support is a first-class goal.

No LinBPQ or BPQ32 GPL code is used. Current status: scaffold only.

## Table of Contents

- [Requirements](#requirements)
- [Setup](#setup)
- [Usage](#usage)
- [Compatibility](#compatibility)
- [License](#license)

## Requirements

- [CMake](https://cmake.org/)
- C17 compiler, such as [Clang](https://clang.llvm.org/) or [GCC](https://gcc.gnu.org/)

No external libraries are required.

## Setup

```sh
cmake -S . -B build
cmake --build build
```

Sanitizer build:

```sh
cmake -S . -B build -DKILONODE_SANITIZE=ON
cmake --build build
```

## Usage

```sh
./build/kilonode
```

Run tests:

```sh
./scripts/test.sh
```

## Compatibility

Compatibility tracking lives in:

- [LinBPQ compatibility matrix](docs/compat/linbpq-compatibility-matrix.md)
- [TNC interface matrix](docs/compat/tnc-interface-matrix.md)

Status values are `planned`, `partial`, `implemented`, and `tested`.

## License

KiloNode is released under the ISC license.


## Mirrors:
