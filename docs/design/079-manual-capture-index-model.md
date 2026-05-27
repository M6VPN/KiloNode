# Manual Capture Index Model

Manual workspaces use `index/captures.index` as a bounded, line-oriented
catalogue.

## Fields

Each capture row stores:

- deterministic ID
- relative file path
- capture method
- source kind
- validation status
- replay status
- added timestamp
- bounded notes

Paths are relative to the workspace. Absolute paths and traversal are rejected.

## Status Transitions

New imports start as `valid`, `invalid`, or `unsupported` after packet capture
validation. Replay updates only the replay status:

- `not-run`
- `pass`
- `fail`
- `unsupported`

FX.25 placeholder captures are catalogued as unsupported until FX.25 decode is
implemented.

## Persistence

The index is rewritten as a complete bounded file. IDs are monotonically
assigned from the largest existing ID plus one and are not reused.
