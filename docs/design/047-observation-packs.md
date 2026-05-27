# Observation Packs

Observation packs group black-box or synthetic compatibility fixtures under one
manifest. They let KiloNode validate fixture provenance, referenced files, and
replayable transcript candidates without executing compatibility targets.

## Manifest Format

```text
# KiloNode observation pack v1
name linbpq-node
subject linbpq
type node-command-observations
source synthetic-placeholder
created 2026-05-27
clean-room true
source-code-used false
fixture help.observation
transcript help.transcript
notes Synthetic placeholders until manually captured observations are added.
```

The parser accepts one key/value pair per line. Comments start with `#`.
Required keys are `name`, `subject`, `type`, `clean-room`, and
`source-code-used`.

`source-code-used` must be `false`. `clean-room` must be `true`.

## References

Fixture and transcript paths are relative to the manifest directory. Absolute
paths, path traversal, duplicate references, and overlong lines are rejected.
Pack validation parses all referenced observations and transcripts.

## Fixture Sources

Fixtures must be marked by source:

- `synthetic-placeholder` for repo-owned test data
- `manual-black-box` for future captured external behaviour

Synthetic fixtures are not authoritative compatibility behaviour.

## Safety

Observation packs do not execute binaries, open transports, inspect source
trees, dispatch TX frames, or infer implementation details. They only validate
and report data already present in the pack.
