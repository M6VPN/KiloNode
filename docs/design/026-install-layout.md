# Install Layout

KiloNode installs binaries through CMake and keeps runtime paths controlled by
config files and service packaging. The code does not require a fixed system
layout for local development.

## CMake Install Paths

Binaries install to `${CMAKE_INSTALL_BINDIR}`.

Example configs install to `${CMAKE_INSTALL_DOCDIR}/examples`.

Manpages install to `${CMAKE_INSTALL_MANDIR}/man1` and
`${CMAKE_INSTALL_MANDIR}/man8`.

Service examples install to `${CMAKE_INSTALL_DOCDIR}/packaging` when
`KILONODE_INSTALL_SERVICE_EXAMPLES` is enabled.

## Recommended Runtime Paths

Linux:

- config: `/etc/kilonode/kilonode.conf`
- runtime: `/run/kilonode`
- data: `/var/lib/kilonode`
- future logs: `/var/log/kilonode`

OpenBSD:

- config: `/etc/kilonode/kilonode.conf`
- runtime: `/var/run/kilonode`
- data: `/var/kilonode`
- rc script: `/etc/rc.d/kilonoded`

FreeBSD and NetBSD:

- config: `/usr/local/etc/kilonode/kilonode.conf`
- runtime: `/var/run/kilonode`
- data: `/var/db/kilonode`
- rc script: `/usr/local/etc/rc.d/kilonoded`

## CMake Options

`KILONODE_DEFAULT_CONFIG_PATH` records the packager-selected config path.

`KILONODE_DEFAULT_RUNTIME_DIR` records the packager-selected runtime directory.

`KILONODE_DEFAULT_DATA_DIR` records the packager-selected data directory.

`KILONODE_INSTALL_SERVICE_EXAMPLES` controls installation of service examples
under the documentation directory.

These options document packaging intent. They do not force root-owned paths for
normal local builds.

## Data Policy

Packagers should create data and runtime directories with ownership and
permissions appropriate for the daemon user.

Examples avoid public network listeners. Public service exposure requires later
authentication and access-control work.

KiloNode does not create system users automatically.
