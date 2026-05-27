# AX.25 Prepared Replay Expectations

M1.46 adds a KiloNode-native expectation file for checking prepared AX.25
diagnostic frames during offline replay.

The format is line-oriented:

```text
# KiloNode AX.25 prepared frame expectations v1
capture kiss-sabm-node.capture {
	prepared-count 1
	prepared 1 kind=UA local=M6VPN-1 remote=N0CALL action=send-ua status=prepared tx-writes=0
	tx-writes 0
}

replay connect-timeout-retry.replay {
	prepared-count 2
	prepared 1 kind=SABM action=send-sabm status=prepared tx-writes=0
	tx-writes 0
}
```

Comments start with `#`. Blocks are keyed by `capture` or `replay` name. The
parser rejects unknown keys, unsafe names, invalid callsigns, invalid frame
kinds, invalid action names, invalid statuses, overlong lines, and any
non-zero `tx-writes` expectation.

Supported checks are:

- `prepared-count N`
- `prepared N kind=KIND action=ACTION status=STATUS`
- optional `local=CALL`, `remote=CALL`, `port=NAME`, `ax25-len=N`
- optional bounded `contains-hex=PREFIX`
- `tx-writes 0`

Timer replay scripts also support inline checks:

```text
expect prepared-count 1
expect prepared kind=SABM action=send-sabm status=prepared
expect tx-writes 0
```
