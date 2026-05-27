# Bench Capture Fixture Model

M1.39 adds a receive-only bench fixture pack for synthetic KISS and AX.25
capture files.

## Manifest

The bench manifest is line-oriented:

```text
name receive-only-bench
type bench-capture-fixtures
source synthetic
clean-room true
source-code-used false
hardware-required false
transmit-required false
fixture kiss-ui-cq.capture
```

Fixture paths are relative to the manifest directory. Absolute paths, path
traversal, duplicate fixtures, missing required fields, and unsafe booleans are
rejected.

## Fixture Safety

Committed fixtures must be synthetic or reviewed manual black-box observations.
They must not require hardware or transmit capability. Capture files are parsed
as hostile input and replayed through the existing packet capture parser.

## Coverage

Bench coverage reports count:

- KISS captures
- raw AX.25 captures represented by AXIP packet-boundary files
- UI frames
- connected-mode setup frames
- supervisory frames
- disconnect frames
- FX.25 placeholders

FX.25 placeholders are planned fixtures only. They are skipped during replay and
counted separately.

## Diagnostic Replay

AX.25 diagnostic replay uses the same manifest and optional expected file
beside it. The replay harness decodes supported captures, feeds AX.25
connected-mode frames into the diagnostics runtime, and reports final counters
and connection-table state.

Sequence captures are present as planned placeholders until multi-frame capture
syntax is added. They must not claim connected-mode replay support before that
parser exists.
