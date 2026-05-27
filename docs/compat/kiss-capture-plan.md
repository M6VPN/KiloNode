# KISS Capture Plan

KISS capture support in M1.28 is offline and packet-boundary only. The tooling
validates text capture files, decodes KISS frames with existing KiloNode KISS
helpers, decodes AX.25 payloads, and compares expected fields.

## Future Manual Workflow

1. Start the black-box target manually in an isolated lab.
2. Capture externally visible KISS frame boundaries.
3. Store each frame in the KiloNode packet capture format.
4. Validate with `kilonode-compat replay-capture`.
5. Keep the original observation data unchanged.

The placeholder script does not open serial, TCP, PTY, Unix, or stdio devices.

## Clean-Room Rule

KISS captures must come from externally visible bytes. Do not inspect GPL source
files or copy source-derived command tables, prompts, message formats, parser
logic, forwarding logic, queue logic, or dispatch logic.

## Deferred Work

- live KISS capture
- multiple frames per capture file
- timing metadata
- replay into KiloNode
- capture review workflow
