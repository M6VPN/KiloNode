# Clean-Room Interop Method

KiloNode compatibility work must use clean-room observations. GPL source code,
command tables, parser logic, prompts, structures, constants, message formats,
forwarding logic, queue logic, and implementation logic are not inputs.

## Allowed Observations

Allowed future observations are externally visible behaviour:

- process stdout and stderr
- command response transcripts
- packet captures at protocol boundaries
- mailbox files created by running a black-box target
- forwarding sessions captured at protocol boundaries
- configuration effects observed by running the target

## Forbidden Inputs

Forbidden inputs include:

- GPL source code
- copied command tables
- copied prompts
- copied parser behaviour
- copied config syntax
- copied structures or constants
- translated implementation logic

## Recording Observations

If behaviour is learned from LinBPQ or BPQ, record it as an observation with:

- date
- target binary and version if known
- environment
- command or frame sent
- response or frame received
- packet capture name if applicable

The observation transcript must describe behaviour only. Implementation remains
KiloNode-native.

## Separation

Observation files and implementation patches should be reviewed separately.
Implementation work must cite KiloNode design decisions and tests, not GPL
source.

## Reviewer Checklist

- No GPL source files were opened.
- No command tables or prompts were copied.
- No parser or forwarding logic was translated.
- Tests use synthetic fixtures or black-box transcripts only.
- New command behaviour is documented as KiloNode-native unless a later
  compatibility pass explicitly validates it.
