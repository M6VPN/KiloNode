# AX.25 Diagnostic Replay

The bench diagnostic replay path runs committed receive-only capture fixtures
through the offline parser and the AX.25 diagnostics runtime. It is for CI and
local bench checks where no radio, TNC, USB sound card, Dire Wolf, or KiloTNC
hardware is available.

Replay flow:

```text
capture file -> KISS/raw AX.25 decode -> AX.25 frame decode -> RX feed -> AX.25 runtime diagnostics -> report
```

UI frames are counted as ignored by connected-mode diagnostics. SABM, SABME,
DISC, DM, UA, RR, RNR, REJ, and I-frame controls are eligible for the AX.25
diagnostic runtime when the fixture and current decoder support them. I-frame
payloads are not delivered to shell or BBS code.

Run the default fixture pack:

```sh
./scripts/bench-rx-replay-diagnostics.sh
```

Run direct commands:

```sh
./build/kilonode-compat replay-bench-diagnostics tests/fixtures/bench/manifest.bench
./build/kilonode-compat replay-bench-diagnostics tests/fixtures/bench/kiss-sabm-node.capture
./build/kilonode-compat bench-diagnostics-report tests/fixtures/bench/kiss-sabm-node.capture
./build/kilonode-compat check-bench-expected tests/fixtures/bench/ax25-diag-replay.expected
```

Reports include parsed frame counts, ignored UI counts, accepted connected
frames, created diagnostic connections, retained frame plans, final connection
state, and attempted TX writes. TX writes must remain zero.

FX.25 placeholder captures are reported as planned or unsupported. They do not
claim FX.25 decode or FEC support.
