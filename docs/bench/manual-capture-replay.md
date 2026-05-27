# Manual Capture Replay

Manual capture replay imports a user-provided receive-only `.capture` file,
validates it, and replays it through the offline AX.25 diagnostics harness.

Workflow:

```sh
./scripts/bench-rx-workspace-init.sh /tmp/kilonode-manual-captures
./scripts/bench-rx-workspace-import.sh tests/fixtures/manual-captures/import-source/kiss-manual-sabm.capture /tmp/kilonode-manual-captures
./scripts/bench-rx-workspace-replay.sh /tmp/kilonode-manual-captures
./scripts/bench-rx-workspace-report.sh /tmp/kilonode-manual-captures
```

Direct commands:

```sh
./build/kilonode-compat manual-import INPUT.capture --workspace /tmp/kilonode-manual-captures --notes manual-import
./build/kilonode-compat manual-validate --workspace /tmp/kilonode-manual-captures
./build/kilonode-compat manual-replay-all --workspace /tmp/kilonode-manual-captures
```

Captures may come from USB sound card plus Dire Wolf TCP KISS, KiloTNC KISS,
serial KISS, TCP KISS, PTY KISS, or Unix socket KISS. Replay itself needs no
hardware.

Replay does not open transports, start Dire Wolf, start KiloTNC, run LinBPQ,
enqueue TX, dispatch frames, expose CONNECT, or deliver payloads to shell or
BBS code.

FX.25 captures can be imported for cataloguing. They replay as unsupported or
planned until FX.25 decode exists.
