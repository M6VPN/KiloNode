# Bench Capture Fixtures

KiloNode keeps synthetic receive-only bench captures under
`tests/fixtures/bench`. They validate packet-boundary KISS and AX.25 capture
tooling without hardware.

These fixtures are not LinBPQ captures and are not sourced from GPL code. They
use generic test callsigns and small deterministic frame bodies.

## Fixtures

| Fixture | Purpose |
| ------- | ------- |
| `kiss-ui-cq.capture` | KISS-framed AX.25 UI frame from `N0CALL` to `CQ` with `CQ TEST`. |
| `kiss-ui-ping-node.capture` | KISS-framed AX.25 UI frame from `N0CALL` to `M6VPN-1` with `PING`. |
| `kiss-sabm-node.capture` | KISS-framed SABM setup frame addressed to `M6VPN-1`. |
| `kiss-disc-node.capture` | KISS-framed DISC frame addressed to `M6VPN-1`. |
| `kiss-rr-node.capture` | KISS-framed RR supervisory frame addressed to `M6VPN-1`. |
| `ax25-ui-cq.capture` | Raw packet-boundary AX.25 UI frame represented with the AXIP capture method. |
| `ax25-sabm-node.capture` | Raw packet-boundary AX.25 SABM frame represented with the AXIP capture method. |
| `fx25-future-placeholder.capture` | Planned FX.25 placeholder. It does not claim FX.25 decode support. |

## Manifest

`tests/fixtures/bench/manifest.bench` lists the committed synthetic fixtures and
states that they are clean-room, hardware-free, and transmit-free.

Replay the pack with:

```sh
./scripts/bench-rx-replay-fixtures.sh
```

The FX.25 placeholder is skipped as planned future work.
