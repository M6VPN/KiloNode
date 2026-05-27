# Black-Box Observation Harness

M1.27 adds an optional harness for recording externally visible behaviour from
black-box compatibility targets. It captures observations as KiloNode-native
observation files and can generate transcript candidates for later review.

This pass does not implement BPQ or LinBPQ compatibility and does not run
LinBPQ automatically.

## Process Runner

`kilonode-compat observe-process` runs only a binary path explicitly supplied
with `--binary`. If a config is needed, it must also be supplied explicitly with
`--config`.

The runner:

- uses `fork` and `execv`, not a shell
- captures stdout and stderr into bounded buffers
- accepts bounded stdin input
- enforces a timeout
- kills and waits for the child on timeout
- rejects missing or non-executable binaries

Tests use a repo-owned helper binary, not LinBPQ.

## TCP Observation

`kilonode-compat observe-tcp` connects only to an explicitly supplied host and
port, sends one bounded line, captures a bounded response, and writes an
observation file.

The helper is meant for future local telnet-style node observations. It does not
implement authentication or command compatibility.

## Capture Limits

Observation parsing and capture are bounded:

- line length: 512 bytes
- observed block: 4096 bytes
- whole observation file: 8192 bytes
- process stdout/stderr: 4096 bytes each
- TCP response: 4096 bytes

Reports escape control characters.

## Safety Rules

- LinBPQ is never run automatically.
- GPL source is never inspected.
- Observation files are hostile input.
- Paths inside observation files are data and are not executed.
- The harness does not open source files under compatibility target trees.
- No real TNC hardware is used by tests.
- No TX dispatch is performed.

## Deferred Work

- live KISS packet capture
- live AXIP observation
- mailbox file observation
- forwarding-session observation
- transcript review workflow
- compatibility implementation
