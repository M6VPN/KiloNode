# Manual Bench Capture Import

Manual receive-only captures can be imported after a bench session. The import
path keeps manual observations outside committed fixtures unless explicitly
allowed.

## Workflow

1. Capture receive-only KISS or packet-boundary AX.25 data.
2. Write it in the KiloNode packet capture format.
3. Validate with `kilonode-compat check-capture`.
4. Import to a local holding directory with `bench-rx-import-capture.sh`.
5. Review metadata, source, safety notes, and payload content.
6. Add to committed fixtures only after review.

## Storage

Use `/tmp/kilonode-manual-captures` or another local working directory for
first-pass imports. Do not overwrite committed fixtures during capture.

## Clean-Room Rules

Manual captures must come from externally visible packet-boundary behaviour.
They must not include copied GPL source, command tables, parser logic,
forwarding logic, queue logic, or prompts converted into implementation code.

## Deferred Work

Future work may add signed capture manifests, richer provenance metadata, and
FX.25-specific capture validation. This pass only validates current KISS and
AX.25 fixture tooling.
