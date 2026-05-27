# LinBPQ Black-Box Plan

This document records a future clean-room plan. M1.26 does not run LinBPQ and
does not inspect GPL source.

Known local paths for a later manual black-box pass:

```text
~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/linbpq
~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/bpq32.cfg.example
```

## Allowed Future Inputs

- process stdout and stderr
- telnet or node session transcripts
- KISS or AXIP packet captures
- externally visible mailbox files created by running the binary
- command response transcripts
- forwarding sessions captured at a protocol boundary

## Forbidden Inputs

- GPL source code
- copied command tables
- copied prompts
- copied structures
- copied parser logic
- copied implementation logic
- copied forwarding or queue logic

## Capture Strategy

Future passes may run the binary manually in an isolated lab, send commands or
frames, and record the external responses as transcript files. Each observation
should include the date, environment, command sent, response received, and any
packet capture reference.

Packet captures should be reduced to externally visible fields needed for
compatibility tests. Implementation code must remain KiloNode-native.

## Node and BBS Transcripts

Later work may capture node and BBS sessions as black-box transcripts. Those
transcripts are observations only. They are not permission to copy prompt text,
command parser structure, mailbox formats, or forwarding implementation.

## Mailbox and Forwarding Observations

Mailbox files and forwarding sessions may be observed only as external outputs
of a running binary. KiloNode import, export, and forwarding designs must remain
separate KiloNode-native implementations.
