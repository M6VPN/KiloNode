# First-Run Checklist

Use this checklist for a first local v0.2-alpha preview run.

- Build KiloNode with `./scripts/build.sh`.
- Validate the hobbyist config:
  `./build/kilonoded --config packaging/examples/kilonode-hobbyist-v0.2-alpha.conf --check-config`.
- Run `./scripts/hobbyist-first-run.sh`.
- Run `./scripts/ax25-no-transmit-check.sh`.
- Run `./scripts/mercury-discovery-check.sh`.
- Start the daemon only with a local config that keeps TX disabled.
- Query `./build/kilonodectl --socket /tmp/kilonode/control.sock status`.
- Query `./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates`.
- Query `./build/kilonodectl --socket /tmp/kilonode/control.sock modem-profiles`.

Stop if any check reports an enabled TX path, CONNECT path, modem auto-start,
or unknown public bind address.
