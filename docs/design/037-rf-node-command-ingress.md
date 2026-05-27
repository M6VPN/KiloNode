# RF Node Command Ingress

M1.24 adds a receive-side RF command ingress path for decoded AX.25 UI
frames. It is KiloNode-native and does not implement BPQ/LinBPQ command
compatibility.

The daemon only considers frames that already decoded as AX.25, use UI frame
control, carry the no-layer-3 PID, fit the configured byte limit, and arrive on
an enabled open port. If `require-node-destination` is true, the destination
must match the node callsign, node alias, or an entry in
`accept-destinations`.

Supported commands are:

| Command | Behaviour |
| ------- | --------- |
| HELP | Lists the RF command names |
| INFO | Returns node identity summary |
| PORTS | Returns a bounded port summary |
| HEARD | Returns a bounded heard-list summary |
| STATS | Returns a bounded daemon counter summary |
| PING | Returns `PONG` |

Arguments are not supported in this pass. Empty, unknown, binary, overlong, and
control-character payloads are recorded as deterministic parse statuses where a
command event is created.

Recent command events are stored in a bounded runtime ring buffer. Events carry
a runtime ID, timestamp, RX event ID when available, port, source, destination,
command name, bounded raw preview, parse status, reply result, and queued TX
frame ID when a reply was queued.

Control queries:

```text
RF STATUS
RF COMMANDS
RF COMMANDS LIMIT <n>
RF COMMANDS PORT <name>
RF COMMANDS FROM <callsign>
RF COMMAND <id>
```

All RF command control queries are read-only and end multiline responses with
`END`.

Deferred work:

- Connected-mode node sessions
- RF BBS access
- BPQ/LinBPQ command compatibility
- NET/ROM routing
- Forwarding
- RF authentication and sysop commands
