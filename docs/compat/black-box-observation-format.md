# Black-Box Observation Format

Observation files record external behaviour from a running black-box target or a
synthetic fixture. The format is KiloNode-native.

## Syntax

- Comments start with `#`.
- Header fields are `key value`.
- Captured output is stored only between `observed-begin` and `observed-end`.
- Unknown keys are rejected.
- Duplicate keys are rejected.
- Line length is bounded.
- Captured output size is bounded.

## Required Fields

```text
name blackbox-node-help
subject linbpq
method telnet
date 2026-05-27
mode node-shell
input HELP
observed-begin
captured output
observed-end
```

Supported methods:

- `process`
- `telnet`
- `tcp-line`
- `packet-capture`
- `mailbox`

Supported modes:

- `process-output`
- `tcp-line`
- `node-shell`
- `bbs-shell`

## Optional Fields

```text
observer M6VPN
source manual-black-box
binary ~/path/to/binary
config ~/path/to/config
connect 127.0.0.1:8010
environment local lab
notes captured from running binary only
packet-capture captures/node-help.pcap
mailbox mailbox-snapshot
result observed
```

Paths in observation files are metadata. The parser does not execute them.

## Example

```text
# KiloNode black-box observation v1
name linbpq-node-help-001
subject linbpq
method telnet
date 2026-05-27
observer M6VPN
source manual-black-box
binary ~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/linbpq
config ~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/bpq32.cfg.example
mode node-shell
connect 127.0.0.1:8010
input HELP
observed-begin
captured output goes here
observed-end
notes captured from running binary only
```

## Safety Rules

- Do not inspect GPL source.
- Do not copy prompts, command tables, parser logic, or implementation logic.
- Do not execute paths from observation files.
- Do not follow links automatically.
- Keep implementation work separate from observation capture.
