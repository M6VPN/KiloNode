# Packet Capture Format

The KiloNode packet capture format stores packet-boundary observations as plain
text. It is intended for synthetic fixtures now and black-box observations later.

## Syntax

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
c0 00 ... c0
frame-end
expect-decode ax25-ui
expect-source N0CALL
expect-destination CQ
expect-kind UI
expect-pid 0xf0
expect-payload-text PING
```

Comments start with `#`. Blank lines are ignored. Packet bytes must be stored in
`frame-begin hex` and `frame-end` blocks.

## Required Fields

KISS captures require:

- `name`
- `method kiss`
- `direction rx` or `direction tx`
- `port`
- one frame block

AXIP and AXUDP captures require:

- `name`
- `method axip` or `method axudp`
- `direction rx` or `direction tx`
- one raw AX.25 frame block

Endpoint fields are optional in M1.28.

## Expectation Fields

- `expect-decode ax25-ui`
- `expect-decode malformed`
- `expect-source CALLSIGN`
- `expect-destination CALLSIGN`
- `expect-kind UI`
- `expect-pid 0xf0`
- `expect-payload-text TEXT`
- `expect-payload-hex HEXBYTES`

## Safety Limits

The parser treats capture files as hostile input. It bounds line length, frame
size, field size, and payload expectations. It does not execute paths, open
referenced files, or infer implementation details from filenames.

## Current Limits

Only one frame per capture file is supported. Live packet capture and pcap input
are not implemented in this pass.
