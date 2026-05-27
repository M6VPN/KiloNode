# TX Dispatch Test Harness

The TX dispatch test harness proves the outbound queue, stored KISS frame bytes,
transport write boundary, status updates, counters, and control diagnostics
without enabling normal RF transmit.

This harness dispatches only to a memory transport. Real KISS transport dispatch
is covered separately in `docs/design/035-real-kiss-tx-dispatch.md` and remains
behind explicit safety gates.

## Memory Transport

`memory-test` is a test-safe transport type. It stores bytes in a bounded
in-process buffer and tracks write calls and bytes written. It never opens
files, sockets, devices, or stdout.

The memory transport supports deterministic test failures:

- bounded capacity
- forced write failure
- forced short write
- reset and readback from tests

## Dispatch Policy

The transmit block now supports:

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
	dispatch-enabled false
	dispatch-test-only true
	dispatch-real-kiss false
	dispatch-max-per-cycle 4
	require-explicit-port-tx true
}
```

Dispatch is disabled by default. For the memory harness, if
`dispatch-enabled true` is set, `enabled` must also be true and
`dispatch-test-only` must remain true.

Dry-run frames may be written only to memory/mock targets. This lets tests prove
KISS bytes and status transitions without RF output.

## Control Commands

The control socket exposes test-only dispatch commands:

```text
TX DISPATCH STATUS
TX DISPATCH RUN
TX DISPATCH RUN PORT <name>
```

Default configs return `ERR tx-dispatch-disabled` for dispatch run commands.
When test-only dispatch is enabled and a memory target exists, `TX DISPATCH RUN`
writes up to `dispatch-max-per-cycle` queued frames to memory and marks results
as sent or failed.

Example response:

```text
OK TX DISPATCH sent=1 failed=0 remaining=0 bytes=32
END
```

## kilonodectl

CLI mappings:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock tx dispatch-status
./build/kilonodectl --socket /tmp/kilonode/control.sock tx dispatch-run
./build/kilonodectl --socket /tmp/kilonode/control.sock tx dispatch-run --port mem0
```

Existing TX diagnostics still apply:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock tx queue
./build/kilonodectl --socket /tmp/kilonode/control.sock tx frame 1
```

## Test-Only Config

`packaging/examples/kilonode-tx-test-only.conf` enables a `memory-test` port
named `mem0`, dry-run control enqueue, and test-only dispatch. The file is not a
normal runtime example and is not for RF.

## Counters

The dispatcher tracks:

- attempts
- sent frames
- failed frames
- policy blocks
- bytes written to memory transports

These counters appear in `TX STATUS` and `TX DISPATCH STATUS`.

## Limitations

- no automatic daemon dispatch loop
- no serial, TCP, PTY, Unix socket, or stdio TX writes
- no retry model
- short writes mark a frame failed
- no TX scheduler

## Deferred Work

- real KISS transport dispatch
- TX scheduler
- PTT and channel busy handling
- retries
- shell TX commands
- RF BBS replies
- connected-mode AX.25
- NET/ROM
- forwarding
