# Real KISS TX Dispatch

M1.23 adds a KiloNode-native dispatch path that can write already queued KISS
frames to configured KISS transports. The path is disabled by default and only
runs when a local admin uses the control socket dispatch command.

This is not exposed through the node shell or BBS shell.

## Dispatch Boundary

Queued TX frames already contain bounded AX.25 bytes and KISS-encoded bytes.
The dispatcher selects a target by port name, checks the transmit policy and
per-port TX gate, then writes the complete KISS frame bytes through the existing
transport write API.

Supported real transport kinds in this pass are:

- serial
- tcp-connect
- tcp-listen accepted client
- pty
- unix-connect
- unix-listen accepted client

`stdio` is blocked for real dispatch. `memory-test` remains test-only.

## Control Trigger

Real dispatch does not run automatically in the daemon loop. It is triggered
only by:

```text
TX DISPATCH RUN
TX DISPATCH RUN PORT <name>
```

The same command continues to support memory-test dispatch when
`dispatch-test-only true` is configured.

## Write Handling

The dispatcher writes a full KISS frame through `kn_transport_write()`.

On success:

- the frame status becomes `sent`
- sent counters increase
- bytes written increase

On write failure:

- the frame status becomes `failed`
- failed counters increase
- no retry is attempted

If a safety gate blocks dispatch, the frame remains queued and the command
returns a deterministic error.

## Diagnostics

Existing TX diagnostics remain available:

```text
TX STATUS
TX QUEUE
TX QUEUE PORT <name>
TX FRAME <id>
TX DISPATCH STATUS
```

M1.23 adds:

```text
TX GATES
TX GATES PORT <name>
```

`TX GATES PORT` reports the configured transport, writable state, per-port
`tx-enabled` flag, final allowed state, and the first deterministic reason for
a block.

## Scope Limits

This is not connected-mode AX.25. It does not implement channel busy handling,
PTT, retries, a scheduler, NET/ROM, BBS forwarding, or RF BBS replies.

## Deferred Work

- automatic TX scheduler
- real transport retry model
- PTT and channel busy detection
- RF watchdog
- duty-cycle policy
- shell TX commands
- RF BBS replies
- connected-mode AX.25
- NET/ROM routing
- forwarding queues
