# KISS and AXIP Observation

KISS and AXIP captures are part of the clean-room compatibility lab. They record
packet-boundary behaviour that can later be compared with other node or BBS
software without reading source code or copying implementation details.

## Synthetic Fixtures

M1.28 adds synthetic fixtures for:

- KISS UI `PING`
- KISS UI `HELP`
- AXIP raw AX.25 UI `PING`

These files are KiloNode-native baseline data. They are not LinBPQ captures and
do not claim BPQ or LinBPQ compatibility.

## Future Black-Box Capture

A later pass may capture externally visible bytes from a running black-box node
process. Valid future sources include KISS frame boundaries, AXIP or AXUDP
packet boundaries, and command session transcripts. Captures must record the
date, environment, observed bytes, input command where applicable, and boundary
where the observation was made.

## Forbidden Inputs

Do not use GPL source files, copied command tables, copied prompts, copied
message formats, copied parser logic, forwarding logic, queue logic, or dispatch
logic. Packet captures are observations only.

## Transcript Candidates

`kilonode-compat capture-to-transcript` can turn an RF UI command payload into a
transcript candidate. The generated file is marked as coming from a packet
capture and remains review data, not implementation code.

## Deferred Work

- multi-frame capture files
- live KISS capture
- live AXIP or AXUDP capture
- endpoint timing reports
- packet replay into KiloNode
- NET/ROM compatibility work
- BBS forwarding compatibility work
