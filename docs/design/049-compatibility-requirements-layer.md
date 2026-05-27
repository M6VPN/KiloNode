# Compatibility Requirements Layer

M1.30 adds a planning layer over observation pack coverage. It converts a
clean-room observation pack into command requirements that can be reviewed
before any compatibility implementation exists.

This layer is planning only. It does not add BPQ/LinBPQ command handling,
connected-mode AX.25, NET/ROM, RF BBS access, forwarding, shell TX commands, or
automatic dispatch.

## Requirements Format

```text
# KiloNode compatibility requirements v1
name linbpq-node-requirements
subject linbpq
source-pack manifest.pack
clean-room true
source-code-used false

requirement HELP {
	status needs-observation
	priority high
	observed synthetic
	mode node-command
	notes Needs manual black-box observations before implementation.
}
```

The parser accepts one header key/value pair per line and bounded
`requirement NAME { ... }` blocks. Paths are relative to the plan file. Absolute
paths, path traversal, duplicate keys, unknown keys, overlong lines, and unknown
values are rejected.

`clean-room` must be `true`. `source-code-used` must be `false`.

## Statuses

- `planned`
- `blocked`
- `needs-observation`
- `ready-for-design`
- `ready-for-implementation`
- `implemented-native`
- `implemented-compatible`
- `out-of-scope`

M1.30 does not mark BPQ/LinBPQ compatibility commands as
`implemented-compatible`.

## Priorities

- `low`
- `medium`
- `high`
- `critical`

CONNECT is critical but blocked until connected-mode AX.25 exists. BBS, NODES,
and ROUTES remain blocked by deferred RF BBS and NET/ROM work.

## Coverage Cross-Checks

`kilonode-compat requirements-coverage` compares a requirements plan with an
observation pack coverage report. It reports commands present in the pack that
lack a requirement entry. The check does not infer command behaviour.
