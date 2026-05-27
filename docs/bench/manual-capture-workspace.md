# Manual Capture Workspace

A manual capture workspace stores user-provided receive-only `.capture` files
outside the committed fixture set. Use it for captures from USB sound card plus
Dire Wolf, KiloTNC, serial KISS, TCP KISS, PTY KISS, Unix socket KISS, and
future reviewed black-box sources.

Recommended location:

```sh
/tmp/kilonode-manual-captures
```

Workspace layout:

```text
manual-captures/
	workspace.manifest
	incoming/
	imported/
	reports/
	replay/
	index/
	notes/
```

Initialize and check:

```sh
./scripts/bench-rx-workspace-init.sh /tmp/kilonode-manual-captures
./build/kilonode-compat manual-workspace-check /tmp/kilonode-manual-captures
```

Safety rules:

- Keep the workspace outside committed fixtures unless a capture has been
  reviewed.
- Do not commit manual captures by default.
- Keep `source-code-used false` and `transmit-required false`.
- Do not store source code, command tables, or implementation notes in capture
  files.
- The workspace tools do not open devices, start daemons, enqueue TX, or
  dispatch frames.
