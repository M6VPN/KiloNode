# Packaging Examples

These configs are KiloNode-native examples for packagers and local installs.

`kilonode-minimal.conf` enables node metadata and local control only.

`kilonode-monitor-only.conf` enables a localhost TCP KISS listener and heard
tracking.

`kilonode-bbs-local.conf` enables the local TCP shell and local BBS store.

`kilonode.conf` is a balanced local example with control, shell, BBS, and a
localhost TCP KISS listener.

`kilonode-tx-test-only.conf` enables memory-only TX dispatch testing.

`kilonode-tx-lab-only.conf` documents real KISS TX gates but still blocks TX by
default.

All examples avoid public network listeners and keep hardware ports disabled by
default.
