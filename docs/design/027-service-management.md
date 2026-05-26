# Service Management

KiloNode ships service files as examples for packagers. They are installed under
the documentation tree by CMake and are not copied into system service
directories automatically.

## systemd

`packaging/systemd/kilonoded.service` runs:

```text
/usr/local/bin/kilonoded --config /etc/kilonode/kilonode.conf --foreground
```

The example uses a dedicated `kilonode` user and group, `RuntimeDirectory`, and
basic hardening settings such as `NoNewPrivileges`, `PrivateTmp`,
`ProtectSystem`, and `ProtectHome`.

Packagers should adjust binary, config, runtime, and data paths for their
package layout.

## OpenBSD rc.d

`packaging/openbsd/kilonoded.rc` is an rc.d-style example intended for
`/etc/rc.d/kilonoded`. It runs the daemon in foreground mode so rc.d can manage
the process.

## FreeBSD rc.d

`packaging/freebsd/kilonoded` is an rc.subr-style example intended for
`/usr/local/etc/rc.d/kilonoded`. The command and config path are configurable
through rc variables.

## NetBSD rc.d

`packaging/netbsd/kilonoded` is an rc.subr-style example. pkgsrc packaging may
choose a different install destination, but the config path defaults to
`/usr/local/etc/kilonode/kilonode.conf`.

## Notes

Service examples do not create users, groups, data directories, or runtime
directories outside service-manager features. Packagers remain responsible for
permissions and ownership.
