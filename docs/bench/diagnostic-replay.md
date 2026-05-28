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
./scripts/ax25-prepared-replay-fixtures.sh
```

Run direct commands:

```sh
./build/kilonode-compat replay-bench-diagnostics tests/fixtures/bench/manifest.bench
./build/kilonode-compat replay-bench-diagnostics tests/fixtures/bench/kiss-sabm-node.capture
./build/kilonode-compat bench-diagnostics-report tests/fixtures/bench/kiss-sabm-node.capture
./build/kilonode-compat check-bench-expected tests/fixtures/bench/ax25-diag-replay.expected
./build/kilonode-compat check-prepared-expect tests/fixtures/bench/prepared-frames.expected
./build/kilonode-compat replay-bench-prepared tests/fixtures/bench/manifest.bench --expect tests/fixtures/bench/prepared-frames.expected
```

Reports include parsed frame counts, ignored UI counts, accepted connected
frames, created diagnostic connections, retained frame plans, final connection
state, prepared diagnostic frame counts, and attempted TX writes. TX writes
must remain zero.

Prepared-frame checks compare the replay result against
`tests/fixtures/bench/prepared-frames.expected`. They inspect only the AX.25
prepared diagnostics queue. They do not bridge frames to the real TX queue.

M1 readiness also runs these replay paths through
`./scripts/m1-readiness-check.sh`. A passing M1 replay result means the
receive/diagnostics fixtures are internally consistent; it does not mean live
connected-mode response TX is implemented.

FX.25 placeholder captures are reported as planned or unsupported. They do not
claim FX.25 decode or FEC support.
