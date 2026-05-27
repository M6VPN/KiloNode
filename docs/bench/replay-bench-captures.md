# Replay Bench Captures

Bench captures can be replayed without radio hardware:

```sh
./scripts/bench-rx-replay-fixtures.sh
./build/kilonode-compat replay-bench-pack tests/fixtures/bench/manifest.bench
```

The replay command validates the manifest, decodes supported KISS and AX.25
fixtures, checks expectations, and prints deterministic pass lines.

The FX.25 placeholder is reported as skipped planned work. A skipped placeholder
is a successful bench-pack replay result because FX.25 FEC and decode are not
implemented.

Run a single capture report with:

```sh
./scripts/bench-rx-capture-report.sh tests/fixtures/bench/kiss-sabm-node.capture
```

Run coverage with:

```sh
./scripts/bench-rx-fixture-status.sh
```
