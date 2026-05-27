# LinBPQ Black-Box Plan

This document records the clean-room black-box plan. M1.27 adds optional
observation tooling, but LinBPQ is still never run automatically and GPL source
is not inspected.

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

M1.28 adds an offline packet-boundary capture format for KISS, AXIP, and AXUDP
observations. These captures are text files containing observed frame bytes and
expected decode fields. They are not pcap files and they do not require live
capture support.

Future KISS observations should store complete KISS frame bytes. Future AXIP and
AXUDP observations should store raw AX.25 payload bytes unless a later document
adds explicit encapsulation metadata.

## Explicit Process Observation Example

This command shape is for a future manual lab only:

```text
./build/kilonode-compat observe-process \
	--binary ~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/linbpq \
	--config ~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/bpq32.cfg.example \
	--name linbpq-startup-001 \
	--mode process-output \
	--input "" \
	--output /tmp/linbpq-startup.observation \
	--timeout 10
```

The user must choose to run it. Tests and scripts do not run this command.

## Explicit TCP Observation Example

```text
./build/kilonode-compat observe-tcp \
	--host 127.0.0.1 \
	--port 8010 \
	--name linbpq-node-help-001 \
	--mode node-shell \
	--input HELP \
	--output /tmp/linbpq-node-help.observation \
	--timeout 10
```

TCP observation captures external session output only. It does not assume
command compatibility or infer implementation details.

## Node and BBS Transcripts

Later work may capture node and BBS sessions as black-box transcripts. Those
transcripts are observations only. They are not permission to copy prompt text,
command parser structure, mailbox formats, or forwarding implementation.

## Mailbox and Forwarding Observations

Mailbox files and forwarding sessions may be observed only as external outputs
of a running binary. KiloNode import, export, and forwarding designs must remain
separate KiloNode-native implementations.

## Packet-Boundary Capture Examples

Synthetic captures can be validated now:

```text
./build/kilonode-compat replay-capture tests/fixtures/compat/kiss-ui-ping.capture
./build/kilonode-compat replay-capture-dir tests/fixtures/compat
```

Future LinBPQ captures must be produced by explicit manual black-box runs. Tests
and scripts do not run the local LinBPQ binary.

## Observation Pack Workflow

M1.29 adds a node observation pack for grouping related node-command
observations:

```text
tests/fixtures/compat/linbpq-node/manifest.pack
```

The current pack contains synthetic placeholders only. A later manual workflow
can add black-box observations by:

1. running the target explicitly in an isolated lab
2. recording command input and external output as `.observation` files
3. adding relative fixture paths to the manifest
4. converting reviewed observations into transcript candidates
5. updating coverage reports

Observation packs do not execute the target binary and do not inspect source.
