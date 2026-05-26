# TX Dry-Run Path

The TX dry-run path lets a local administrator exercise outbound AX.25 UI frame
building, KISS encoding, queue insertion, and control-socket diagnostics without
writing bytes to a TNC transport.

This pass does not transmit packets.

## Policy Gates

Dry-run enqueue is disabled unless all required transmit policy gates are set:

```text
transmit {
	enabled true
	dry-run true
	max-queued 128
	max-payload-bytes 256
	payload-preview-bytes 80
	allow-ui true
	allow-control-enqueue true
	allow-shell-enqueue false
}
```

`allow-shell-enqueue` remains false in normal examples. No node shell or BBS
shell TX command is implemented in this pass.

If `dry-run false` is combined with a local enqueue gate, config validation
rejects the file because runtime dispatch is not implemented.

## Control Command

The native control command is:

```text
TX DRYRUN UI PORT <port> FROM <source> TO <dest> [VIA <path>] TEXT <payload>
```

Example:

```text
TX DRYRUN UI PORT kiss0 FROM M6VPN-1 TO CQ TEXT hello
```

The optional VIA path is comma-separated:

```text
TX DRYRUN UI PORT kiss0 FROM M6VPN-1 TO CQ VIA WIDE1-1,WIDE2-1 TEXT hello
```

The command validates:

- transmit policy gates
- configured port exists, is enabled, and is open
- source and destination callsigns
- optional digipeater path
- payload length
- queue capacity

On success, the command enqueues a dry-run frame and returns the assigned frame
ID.

## kilonodectl

The CLI maps to the control command:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock tx dryrun-ui --port kiss0 --from M6VPN-1 --to CQ --text "hello from dry run"
./build/kilonodectl --socket /tmp/kilonode/control.sock tx dryrun-ui --port kiss0 --from M6VPN-1 --to CQ --via WIDE1-1,WIDE2-1 --text "hello"
```

Queue diagnostics remain read-only:

```sh
./build/kilonodectl --socket /tmp/kilonode/control.sock tx queue
./build/kilonodectl --socket /tmp/kilonode/control.sock tx frame 1
```

## No Transport Writes

The dry-run helper only builds a frame and calls the in-memory queue API. It has
no transport pointer, no file descriptor, and no call path to
`kn_transport_write`.

## Safety Limits

Payloads use explicit lengths and are bounded by transmit policy. The queue is
bounded and rejects new frames when full. Payload previews are bounded and
diagnostic only.

## Deferred Work

- actual dispatch
- local shell TX command
- RF node replies
- RF BBS replies
- connected-mode AX.25
- NET/ROM
- forwarding
- LinBPQ/BPQ interop tests
