# Compatibility Lab Harness

M1.26 adds a clean-room compatibility lab for RF command behaviour. The lab is
for KiloNode-native synthetic transcripts now and future black-box observations
later.

The harness does not implement BPQ or LinBPQ command compatibility. It does not
run LinBPQ, read GPL source, inspect command tables, copy prompts, or translate
implementation logic.

## Transcript Format

Transcripts are small text files with one key and value per line. Comments start
with `#`. The current mode is `rf-ui`, which describes one inbound AX.25 UI
payload addressed to a node.

Required fields for `rf-ui`:

- `name`
- `mode`
- `node`
- `port`
- `source`
- `destination`
- `pid`
- `input`
- `expect-event`
- `expect-reply`

Expected result fields describe externally visible behaviour, such as the
parsed command, event status, reply contents, whether a reply was queued, and
whether dispatch must not occur.

## Replay Engine

`kilonode-compat replay-transcript` parses one transcript, builds a synthetic
receive event, runs it through the RF command ingress path, captures the command
event, captures any queued UI reply, and compares the result with the expected
fields.

Replay is synthetic only:

- no real TNC hardware
- no real transports
- no TX dispatch
- no network service
- no LinBPQ execution

## Reports

Reports are deterministic text. They include the transcript name, mode,
pass/fail result, observed command, observed status, reply queued flag, TX frame
ID if any, escaped reply preview, and mismatch lines.

## CLI

Commands:

- `kilonode-compat check-transcript PATH`
- `kilonode-compat replay-transcript PATH`
- `kilonode-compat replay-dir PATH`
- `kilonode-compat report-transcript PATH`

`replay-dir` runs `*.transcript` files in sorted order.

## Fixtures

The fixtures under `tests/fixtures/compat` are synthetic KiloNode-native
baselines. They are not LinBPQ observations and do not claim compatibility.

## Deferred Work

- black-box LinBPQ execution harness
- packet capture ingestion
- connected-mode AX.25 transcripts
- NET/ROM transcripts
- RF BBS session transcripts
- forwarding observations
- BPQ/LinBPQ compatibility implementation
