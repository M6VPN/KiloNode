# Packaging Policy

KiloNode packaging should be simple, repeatable, and clean-room. Packages must
not add GPL dependencies or copy BPQ or LinBPQ behavior, config syntax, command
syntax, prompts, message formats, or implementation details.

## Goals

Packaging should provide:

- installed binaries
- example configs
- service-manager examples
- manpages
- documented runtime paths

No runtime dependencies are added in this pass.

## Packager Responsibilities

Packagers should:

- create a `kilonode` user and group if the platform package model supports it
- create runtime and data directories
- install a config file
- set ownership and permissions
- decide whether service files belong in system service directories
- adapt paths to platform policy

## Local Install Scripts

`scripts/install-local.sh` installs into an explicit prefix through CMake.

`scripts/uninstall-local.sh` removes known installed files from an explicit
prefix and leaves data directories untouched.

`scripts/check-packaging.sh` checks service examples, config examples, manpage
sections, and packaging safety assumptions.

## Deferred Packaging

Deferred work includes deb packages, rpm packages, apk packages, pkgsrc,
OpenBSD ports, FreeBSD ports, release tarballs, signed releases, and binary
reproducibility work.
