# Command Profile Model

Command profiles describe external command shape at a high level. They are not
parser code, command tables, prompt definitions, or output templates.

Profiles can be generated from observation packs to create a conservative
planning baseline. Exact output text must come from observation references, not
from copied implementation material.

## Profile Format

```text
# KiloNode command profiles v1
name linbpq-node-command-profiles
subject linbpq
clean-room true
source-code-used false

command HELP {
	category informational
	transport telnet
	args none
	reply one-or-more-lines
	stateful false
	requires-connected-mode false
	compat-status needs-observation
}
```

Unknown header keys, unknown block keys, duplicate commands, overlong fields,
invalid booleans, and unsupported enum values are rejected.

## Categories

- `informational`
- `session`
- `bbs`
- `routing`
- `sysop`
- `unknown-handling`

## Transports

- `local-shell`
- `rf-ui`
- `connected-ax25`
- `netrom`
- `telnet`

## Argument And Reply Classes

Argument classes are `none`, `optional`, `required`, and `free-text`.

Reply classes are `none`, `one-line`, `one-or-more-lines`, and
`session-transition`.

## Compatibility Status

Profiles support `needs-observation`, `planned`, `blocked`, `native-only`, and
`ready-for-design`.

This pass keeps BPQ/LinBPQ compatibility in planning states only.
