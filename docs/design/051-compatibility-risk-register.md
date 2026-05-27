# Compatibility Risk Register

M1.30 adds a default risk register for clean-room node compatibility planning.
The register is reported by `kilonode-compat risk-report` and is not an
implementation checklist.

## Risk Categories

- GPL contamination risk
- prompt and output copying risk
- command ambiguity risk
- connected-mode AX.25 prerequisite
- NET/ROM prerequisite
- BBS mailbox format unknown
- forwarding protocol unknown
- unsafe transmit risk
- user and sysop authentication risk
- regression against KiloNode-native shell behaviour

## Severities

- `low`
- `medium`
- `high`
- `critical`

## Statuses

- `open`
- `mitigated`
- `accepted`
- `deferred`

## Mitigation Strategy

Compatibility work must stay observation-driven. A future pass may record
externally visible behaviour from a running binary, network session, packet
capture, or produced data file. It must not inspect GPL source or copy command
tables, prompts, parser logic, message formats, routing logic, forwarding
logic, queue logic, or dispatch logic.

Blocked prerequisites remain explicit. CONNECT needs connected-mode AX.25.
NODES and ROUTES need NET/ROM design. RF BBS commands need RF BBS access and
mailbox compatibility observations. Real TX remains behind the safety gates
added before this pass.
