# Portability Policy

KiloNode targets portable C17 with POSIX APIs. Linux is the primary platform.
OpenBSD, FreeBSD, and NetBSD are strong targets. macOS is a possible later
target.

## Platform Targets

Linux builds are expected to pass with GCC and Clang.

OpenBSD, FreeBSD, and NetBSD should remain practical targets. Platform-specific
code must be isolated and guarded when an API is not portable.

macOS support is not a primary target yet, but code should avoid unnecessary
Linux-only assumptions.

## POSIX Assumptions

The project currently uses POSIX APIs for sockets, Unix domain sockets, poll,
termios serial ports, PTYs, file operations, and advisory locking.

Transport code is the main platform-sensitive area:

- serial KISS uses `termios`
- PTY KISS uses `posix_openpt`, `grantpt`, `unlockpt`, and `ptsname`
- Unix socket KISS uses `AF_UNIX`
- TCP KISS uses POSIX sockets and `getaddrinfo`
- daemon multiplexing uses `poll`
- store locking uses advisory file locks

Unsupported APIs should fail cleanly at compile time or runtime with clear
errors.

## Dependency Policy

KiloNode must not add GPL dependencies. Compatibility work must remain
clean-room. BPQ and LinBPQ behavior may be studied later only through black-box
tests, packet captures, config examples, and externally visible behavior.

## Portability Checks

`scripts/check-portability.sh` catches common hazards such as unsafe string
functions, GNU-only functions, Linux-only headers, and accidental sudo use in
scripts. It is a guardrail, not a complete portability audit.
