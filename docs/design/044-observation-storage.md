# Observation Storage

Black-box observations are stored as KiloNode-native text files. They preserve
the original captured output inside an explicit observed block and keep metadata
outside the block.

## Format

Observation files use key/value fields followed by:

```text
observed-begin
captured output
observed-end
```

The captured block is data only. It is not parsed as command tables, prompts, or
implementation logic.

## Storage Recommendations

Store synthetic fixtures under `tests/fixtures/compat`. Store future real
black-box observations in a separate review directory, keeping packet captures
and mailbox snapshots outside the source tree unless they are small and cleared
for publication.

## Transcript Conversion

`make-transcript-from-observation` creates a transcript candidate. The original
observation remains unchanged. The candidate is marked:

```text
source black-box-observation
```

Generated candidates are review inputs only. They must not be converted into C
tables, parser logic, prompts, or message formats.

## Reports

Reports show metadata and an escaped preview. They do not print unbounded
captured output.

## Deferred Modes

- KISS capture
- AXIP capture
- mailbox observation
- forwarding observation
