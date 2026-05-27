# LinBPQ Node Observation Pack

The current `linbpq-node` observation pack is a synthetic placeholder pack used
to test KiloNode compatibility tooling.

Pack path:

```text
tests/fixtures/compat/linbpq-node/manifest.pack
```

## Contents

Synthetic observations:

- HELP
- INFO
- PORTS
- USERS
- UNKNOWN

Synthetic transcript candidates:

- HELP
- INFO
- UNKNOWN

These fixtures are not captured from LinBPQ and do not claim LinBPQ
compatibility.

## Future Manual Captures

Later passes may add `manual-black-box` observations captured from externally
visible sessions against a running process. Each capture must record date,
environment, command input, observed output, and source as black-box data.

Do not inspect GPL source files or copy source-derived command tables, prompts,
message formats, parser logic, forwarding logic, queue logic, or dispatch logic.
