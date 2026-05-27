# Manual Capture Index

Manual workspaces store capture metadata in `index/captures.index`.

Index rows are line-oriented:

```text
capture id=1 file=imported/kiss-manual-sabm.capture method=kiss source=manual status=valid replay=pass added=1710000000 notes=manual-import
```

Fields:

- `id`: deterministic numeric ID, never reused inside the index.
- `file`: relative path under the workspace.
- `method`: capture method from the packet capture parser.
- `source`: `manual`, `synthetic`, or `black-box`.
- `status`: `unchecked`, `valid`, `invalid`, or `unsupported`.
- `replay`: `not-run`, `pass`, `fail`, or `unsupported`.
- `added`: import timestamp.
- `notes`: bounded operator note with unsafe whitespace folded.

Missing or corrupt referenced captures are reported during validation. Absolute
paths and path traversal are rejected.

List and summarize:

```sh
./scripts/bench-rx-workspace-list.sh /tmp/kilonode-manual-captures
./build/kilonode-compat manual-summary --workspace /tmp/kilonode-manual-captures
```
