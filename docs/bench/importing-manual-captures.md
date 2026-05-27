# Importing Manual Bench Captures

Manual bench captures can be imported later from receive-only KISS or AX.25
packet-boundary observations. Typical sources include Dire Wolf fed by a USB
sound card, KiloTNC, serial KISS, TCP KISS, PTY KISS, or Unix socket KISS.

Manual captures are observations. They are not implementation sources. Do not
copy LinBPQ/BPQ32 source, comments, command tables, parser logic, prompts,
message formats, forwarding logic, or queue logic into capture files or docs.

## Import Command

Use a local destination outside committed fixtures by default:

```sh
./scripts/bench-rx-import-capture.sh /path/to/manual.capture /tmp/kilonode-manual-captures
```

The import helper:

- requires a `.capture` input
- validates the capture when `build/kilonode-compat` exists
- rejects path traversal in the destination
- refuses `tests/fixtures/bench` unless `--allow-repo-fixture` is explicit
- preserves the original file
- does not open devices or start bench software

## Review Before Committing

Before adding manual captures to committed fixtures, verify:

- capture source and date are documented
- transmit was disabled during capture
- no payload contains private data
- no GPL source text or derived implementation details are present
- replay output is deterministic
