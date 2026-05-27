# Bench Replay Report Model

M1.40 adds deterministic AX.25 diagnostic replay reports for receive-only bench
captures.

## Result Fields

Each replay result records:

- capture name and method
- parsed frame count
- decoded frame count
- ignored UI count
- accepted connected-frame count
- ignored and malformed frame counts
- diagnostic connections created
- final connection count and state
- retained frame-plan count
- attempted TX writes
- pass, fail, or planned placeholder status

TX write count is part of every result and must be zero.

## Expected File

Expected replay data can live beside the bench manifest in
`ax25-diag-replay.expected`.

```text
capture kiss-sabm-node.capture {
	frames-parsed 1
	connections-created 1
	final-connections 1
	expected-state connected
	tx-writes 0
}
```

Unknown keys, unsafe capture paths, invalid integers, absolute paths, and path
traversal are rejected. Missing capture blocks are allowed so new fixtures can
start in report-only mode.

## Ordering

Manifest replay follows manifest order. Directory replay sorts capture paths
before replay. Reports use stable single-line records so they are suitable for
CI logs and shell scripts.

## Mismatches

Expectation mismatches mark the capture as failed and add bounded mismatch
lines. Reports do not include payload bytes or unbounded capture data.
