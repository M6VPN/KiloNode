# M1 Compatibility Audit

M1 compatibility work is clean-room lab infrastructure, not LinBPQ command
compatibility.

## Clean-Room Policy

The project uses:

- Public protocol references under `docs/references`.
- Project-owned synthetic fixtures.
- Project-owned observations and plans.
- Black-box observation formats for future manual work.

The known local LinBPQ binary and example config are future inputs only. M1
scripts do not run them and do not inspect GPL source.

## Implemented Lab Tooling

- Synthetic RF command transcripts.
- KISS and AXIP capture fixtures.
- Bench capture manifest and replay.
- AX.25 diagnostic replay.
- Prepared-frame replay assertions.
- Timer replay scripts.
- Manual capture workspace and import flow.
- Observation pack validation.
- Requirements, command profiles, coverage, and risk reports.

Run:

```sh
./scripts/m1-compatibility-audit.sh
```

## Current Compatibility Status

KiloNode-native features exist for local shell, local BBS storage, control
queries, receive-only monitoring, and diagnostic AX.25 scaffolds.

LinBPQ/BPQ command compatibility remains planned. The M1 fixtures document
intent and lab shape only. CONNECT, NODES, ROUTES, RF BBS, forwarding, sysop,
and password compatibility remain blocked or planned.

## Remaining Work Before Actual LinBPQ Compatibility

- Add reviewed black-box observations from a running LinBPQ instance.
- Keep GPL source out of the implementation workflow.
- Expand receive-only captures around connected-mode setup and teardown.
- Define a compatibility mode separate from KiloNode-native commands.
- Finish real connected-mode session and TX safety milestones before live RF
  interop.
