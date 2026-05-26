# Config Format

KiloNode uses a native config format. It is not BPQ or LinBPQ syntax. Future
BPQ-style import may be added as a compatibility tool, but the native parser is
kept separate.

Comments start with `#`. Whitespace is insignificant outside quoted strings.
Strings may be quoted when they contain spaces.

Node block:

```text
node {
	callsign M6VPN-1
	alias KILON
	location "Test node"
}
```

`callsign` is required and uses the existing callsign and SSID validation.
`alias` and `location` are optional.

Control block:

```text
control {
	enabled true
	path /tmp/kilonode/control.sock
}
```

The control block is optional. If omitted, control is disabled. If enabled,
`path` is required.

Heard block:

```text
heard {
	enabled true
	max-entries 256
}
```

The heard block is optional. If omitted, heard tracking is enabled with a
256-entry limit. `max-entries` must be between 1 and 256.

Shell block:

```text
shell {
	enabled true
	host 127.0.0.1
	port 8010
	max-clients 4
	banner "KiloNode test shell"
}
```

The shell block is optional. If omitted, the local node shell is disabled. If
enabled, `host` and `port` are required. `max-clients` must be between 1 and 16.
`banner` is optional.

Port block:

```text
port kiss0 {
	type tcp-listen
	host 127.0.0.1
	port 8001
	max-frame 2048
	enabled true
}
```

Supported port types:

- `stdio`
- `tcp-connect`
- `tcp-listen`
- `serial`
- `pty`
- `unix-connect`
- `unix-listen`

Validation rules:

- Unknown top-level blocks are rejected.
- Unknown keys are rejected.
- Duplicate keys are rejected.
- Duplicate control blocks are rejected.
- Duplicate heard blocks are rejected.
- Duplicate shell blocks are rejected.
- Port names must be unique.
- Disabled ports parse but are not opened.
- `max-frame` must be between `KN_CONFIG_MAX_FRAME_MIN` and
  `KN_CONFIG_MAX_FRAME_MAX`.
- Port type-specific required keys must be present.

Required port keys:

| Type | Required keys |
| ---- | ------------- |
| `stdio` | `type` |
| `pty` | `type` |
| `tcp-connect` | `type`, `host`, `port` |
| `tcp-listen` | `type`, `host`, `port` |
| `serial` | `type`, `device`, `baud` |
| `unix-connect` | `type`, `path` |
| `unix-listen` | `type`, `path` |
