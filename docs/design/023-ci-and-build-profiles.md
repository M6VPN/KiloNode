# CI and Build Profiles

KiloNode uses CMake for local builds and GitHub Actions for CI. CI is focused
on deterministic tests, compiler warnings, sanitizer checks, and simple
portability checks.

## Workflows

`CI` runs on pushes to `main` and pull requests. It builds Debug profiles with
GCC and Clang, enables warnings as errors, runs header and portability checks,
and runs the test suite.

`Sanitizers` runs on pushes to `main`, pull requests, and manual dispatch. It
builds Clang and GCC sanitizer jobs with AddressSanitizer and
UndefinedBehaviorSanitizer.

`Portable Build` runs on pull requests and manual dispatch. It runs a strict
C17 build and a hardened Release build on Ubuntu. BSD platforms are documented
targets, not hosted CI targets in this pass.

## Local Scripts

`./scripts/build.sh` configures `build/` as a Debug build and builds it.

`./scripts/test.sh` configures, builds, and runs all deterministic tests.

`./scripts/build-sanitize.sh` configures `build-sanitize/` with ASan and UBSan,
builds it, and runs tests.

`./scripts/build-release.sh` configures `build-release/` as a hardened Release
build.

`./scripts/check-headers.sh` checks required project headers in source, CMake,
shell, and example config files.

`./scripts/check-portability.sh` performs grep-based checks for common unsafe
or non-portable patterns. It is not a full static analyzer.

## CMake Options

`KILONODE_WARNINGS_AS_ERRORS` turns supported compiler warnings into errors.
It is off by default for local builds and enabled in CI.

`KILONODE_SANITIZE` remains as a compatibility option and enables ASan and
UBSan.

`KILONODE_SANITIZE_ADDRESS`, `KILONODE_SANITIZE_UNDEFINED`, and
`KILONODE_SANITIZE_LEAK` can be enabled individually.

`KILONODE_HARDENING` enables the default hardening set.

`KILONODE_FORTIFY`, `KILONODE_STACK_PROTECTOR`, `KILONODE_RELRO_NOW`, and
`KILONODE_PIE` can be enabled individually.

## Warning Policy

Warnings are kept clean under GCC and Clang. CI enables warnings as errors so
new warnings are caught before merge. Local builds leave `-Werror` off by
default so compiler differences do not block quick development.
