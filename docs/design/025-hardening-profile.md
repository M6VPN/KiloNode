# Hardening Profile

KiloNode has optional CMake hardening settings for local release builds and CI.
The hardening profile is best-effort because compiler and linker support varies
by platform.

## Options

`KILONODE_HARDENING` enables the default hardening set.

`KILONODE_FORTIFY` defines `_FORTIFY_SOURCE=2` for optimized builds.

`KILONODE_STACK_PROTECTOR` enables `-fstack-protector-strong` when supported.

`KILONODE_PIE` enables position independent executable flags when supported.

`KILONODE_RELRO_NOW` enables ELF `relro` and `now` linker flags when supported.

## Build

Use:

```sh
./scripts/build-release.sh
```

or:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DKILONODE_HARDENING=ON
cmake --build build-release
```

## Limits

Hardening flags do not replace input validation, tests, fuzzing, privilege
separation, or runtime sandboxing.

Deferred work includes privilege separation, chroot or jail support,
pledge/unveil on OpenBSD, seccomp on Linux, systemd sandboxing, and Capsicum on
FreeBSD.
