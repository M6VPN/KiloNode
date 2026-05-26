# RF Transmit Queue

The transmit queue is a bounded, in-memory queue for prepared outbound AX.25
frames. It is daemon-owned and exists so future RF features can share one safe
queue model.

This pass does not dispatch queued frames to transports.

## Config

The native config format supports:

```text
transmit {
	enabled false
	dry-run true
	max-queued 128
	max-payload-bytes 256
	payload-preview-bytes 80
	allow-ui false
	allow-control-enqueue false
	allow-shell-enqueue false
}
```

The block is optional. If omitted, transmit remains disabled, dry-run remains
enabled, and UI frame enqueue is not allowed.

## Queue Limits

Defaults:

- max queued frames: 128
- max payload bytes: 256
- payload preview bytes: 80

The queue rejects new frames when full. It does not evict old frames, grow
without bounds, or persist across daemon restarts.

## Policy

Transmit policy is deliberately conservative:

- disabled transmit rejects enqueue requests
- dry-run transmit never writes to a transport
- `allow-ui false` rejects UI frame enqueue through policy helpers
- `allow-control-enqueue false` rejects control-socket dry-run enqueue
- `allow-shell-enqueue false` keeps local shell enqueue disabled

Tests can construct frames directly where needed, but runtime user surfaces do
not enqueue transmit frames in this pass.

## Control Queries

The control socket exposes read-only commands:

- `TX STATUS`
- `TX QUEUE`
- `TX QUEUE PORT <name>`
- `TX FRAME <id>`

There are no write commands. `TX SEND`, `TX ENQUEUE`, and `TX CLEAR` are not
implemented.

## Deferred Work

- actual dispatch to TNC transports
- shell TX commands
- RF BBS replies
- AX.25 connected-mode state machine
- NET/ROM routing
- forwarding queues
- transmit counters in daemon stats
- BPQ/LinBPQ interop tests
