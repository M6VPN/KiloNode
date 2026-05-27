# Manual Capture Workspace

M1.41 adds a local workspace for user-provided receive-only captures. The
workspace keeps manual data outside committed fixtures by default and gives the
operator an explicit import, validation, replay, and report path.

## Structure

The workspace contains:

- `workspace.manifest`
- `incoming/`
- `imported/`
- `reports/`
- `replay/`
- `index/`
- `notes/`

Only files under the requested workspace root are created or updated. Empty
paths, root paths, path traversal, and unsafe relative paths are rejected.

## Manifest

The manifest records clean-room status:

```text
name manual-rx-captures
type manual-capture-workspace
clean-room true
source-code-used false
transmit-required false
```

`source-code-used true` and `transmit-required true` are rejected.

## Clean-Room Policy

Manual captures are observations. They must not include copied source code,
command tables, parser logic, or implementation notes from GPL projects.
