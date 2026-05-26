# Node Shell

The node shell is a local TCP diagnostic shell for `kilonoded`. It exposes
current daemon state to a telnet or netcat client. It does not transmit packets
and it is not BPQ command compatibility.

Config block:

```text
shell {
	enabled true
	host 127.0.0.1
	port 8010
	max-clients 4
	banner "KiloNode test shell"
}
```

The block is optional. If omitted, the shell is disabled. If enabled, `host` and
`port` are required. The example binds to `127.0.0.1` only.

The listener is integrated into the daemon poll loop. It accepts local TCP
clients, sends the configured banner, prints a short help hint, and then shows
the `KILONODE>` prompt. `max-clients` is bounded and enforced at accept time.

Each session stores:

- connection fd
- bounded receive line buffer
- connected timestamp
- remote address
- command count
- closed state

Input lines are bounded to 512 bytes. Overlong lines are discarded, an error is
returned, and the session remains open. Control characters are replaced before
they reach command parsing or user-facing output.

Commands:

| Command | Behaviour |
| ------- | --------- |
| `HELP` | List available shell commands |
| `INFO` | Show callsign, alias, location, and version |
| `PORTS` | Show configured port enabled/open state |
| `HEARD` | Show current heard list |
| `HEARD PORT <name>` | Show heard entries for one port |
| `USERS` | Show connected local shell users |
| `STATS` | Show daemon counters |
| `BYE` | Close the session |
| `QUIT` | Close the session |

Deferred work:

- authentication
- public access mode
- RF connect command
- NET/ROM node commands
- BBS handoff
- sysop commands
- rate limiting
- idle timeouts
