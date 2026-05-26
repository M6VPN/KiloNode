# Control Plane

The control plane is local-only in this pass. `kilonoded` creates a Unix-domain
stream socket, and `kilonodectl` sends one read-only command per connection.
There is no remote TCP control.

The protocol is line-oriented:

```text
COMMAND\n
```

Responses are text lines:

```text
OK ...
ERR ...
```

Commands that return multiple lines end with:

```text
END
```

Supported commands:

- `PING`
- `VERSION`
- `STATUS`
- `PORTS`
- `STATS`
- `HELP`
- `QUIT`

The configured socket path controls local access through filesystem
permissions. The daemon sets the socket path to owner-only permissions where
the platform allows it. Parent directories are not created automatically.

The control plane is read-only. It can report daemon status, port state, and
runtime counters, but it cannot reload config, transmit packets, modify ports,
or run sysop commands.

Deferred work:

- Config reload
- Admin authentication
- Remote control
- Packet transmit commands
- BBS controls
- NET/ROM controls
