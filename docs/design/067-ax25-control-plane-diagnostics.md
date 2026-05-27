# AX.25 Control Plane Diagnostics

M1.36 exposes read-only diagnostics for the AX.25 connected-mode scaffold through the existing daemon control socket.

The control plane reports the daemon-owned AX.25 runtime, its parameter snapshot, connection-table records, retained frame plans, and counters. It does not create, clear, connect, disconnect, enqueue, dispatch, or deliver any AX.25 session data.

## Control Commands

Supported control protocol commands:

| Command                  | Purpose |
|--------------------------|---------|
| `AX25 STATUS`            | Show runtime enable flags, connection count, table limit, and diagnostics flag. |
| `AX25 PARAMS`            | Show the current AX.25 connected-mode parameter snapshot. |
| `AX25 CONNECTIONS`       | List bounded connection-table summaries. |
| `AX25 CONNECTIONS PORT name` | List connection summaries for one validated port name. |
| `AX25 CONNECTION id`     | Show one retained connection diagnostic record by deterministic 1-based ID. |
| `AX25 COUNTERS`          | Show runtime diagnostic counters. |

All multiline responses end with `END`. Empty lists return an `OK` response with `count=0`.

There are no `AX25 CONNECT`, `AX25 DISCONNECT`, or `AX25 CLEAR` commands in this pass.

## CLI Commands

`kilonodectl` maps these read-only commands:

| CLI                                      | Control protocol |
|------------------------------------------|------------------|
| `ax25 status`                            | `AX25 STATUS` |
| `ax25 params`                            | `AX25 PARAMS` |
| `ax25 connections`                       | `AX25 CONNECTIONS` |
| `ax25 connections --port kiss0`          | `AX25 CONNECTIONS PORT kiss0` |
| `ax25 connection 1`                      | `AX25 CONNECTION 1` |
| `ax25 counters`                          | `AX25 COUNTERS` |

The CLI prints daemon responses unchanged and returns non-zero for invalid arguments, connection failures, or `ERR` responses.

## Response Shape

Example empty runtime:

```text
OK AX25 STATUS enabled=false connected_mode=false connections=0 max_connections=32 diagnostics=true
END
```

Example connection list:

```text
OK AX25 CONNECTIONS count=1
AX25 CONN id=1 port=kiss0 local=M6VPN-1 remote=N0CALL state=connected frames=1 plans=1 last=rx-sabm
END
```

Connection details expose bounded counters and retained frame plans:

```text
OK AX25 CONNECTION id=1
AX25 CONN id=1 port=kiss0 local=M6VPN-1 remote=N0CALL state=connected created=1710000000 last=1710000000 rx_sabm=1 rx_i=0 rx_s=0 protocol_errors=0 plans=1
AX25 PLAN index=0 kind=UA source=M6VPN-1 dest=N0CALL
END
```

## Safety

Port filters and connection IDs are validated before use. Rows and output buffers are bounded by the existing control response limits.

Raw payload bytes are not exposed. Frame plans are diagnostic summaries only. They are not sent to the TX queue and are not dispatched.

## Scope

This is not a full AX.25 connected-mode implementation. It is a control-plane view into the scaffold and unit-test injection path created in earlier passes.

FX.25 remains outside this diagnostic plane. Future FX.25 decode may unwrap bytes before AX.25 event creation, and future FX.25 encode may wrap AX.25 frames after action mapping.
