# Packet Capture Format

M1.28 adds a KiloNode-native packet-boundary capture format for synthetic and
future black-box KISS, AXIP, and AXUDP observations.

The format is text based, line oriented, and does not require pcap or libpcap.
It records externally visible packet bytes only. It is not a BPQ or LinBPQ
format and does not describe implementation internals.

## Format

Capture files use comments starting with `#`, key/value header lines, and one
hex frame block:

```text
# KiloNode packet capture v1
name kiss-ui-ping
subject synthetic
method kiss
date 2026-05-27
observer M6VPN
mode packet-boundary
direction rx
port kiss0
timestamp 1710000000
frame-begin hex
c0 00 86 a2 40 40 40 40 60 9c 60 86 82 98 98 61 03 f0 50 49 4e 47 c0
frame-end
expect-decode ax25-ui
expect-source N0CALL
expect-destination CQ
expect-kind UI
expect-pid 0xf0
expect-payload-text PING
```

Supported methods are `kiss`, `axip`, and `axudp`. KISS captures store complete
KISS frames. AXIP and AXUDP captures store raw AX.25 frame payload bytes, not IP
headers.

## Expectations

The replay command can compare decoded fields against optional expectations:

- `expect-decode`
- `expect-source`
- `expect-destination`
- `expect-kind`
- `expect-pid`
- `expect-payload-text`
- `expect-payload-hex`

Expectation mismatches produce deterministic text reports and non-zero replay
results.

## Limits

The parser rejects overlong lines, malformed hex, missing frame blocks, nested
frame blocks, duplicate singleton fields, oversized frames, and unsupported
methods. The current pass supports one frame per file. Multi-frame captures are
deferred.

## Reports

`kilonode-compat decode-capture`, `replay-capture`, and `capture-report` print
the capture name, method, pass/fail result, decoded callsigns, frame kind, PID,
payload preview, and mismatch details.

## Scope

This pass does not implement live capture, AXIP routing, NET/ROM, connected-mode
AX.25, BBS forwarding, RF BBS access, or BPQ/LinBPQ command compatibility.
