# VARA External Modem Roadmap

VARA FM and VARA HF are planned external modem adapter targets for KiloNode.
They are not implemented in the current release.

Future KiloNode support should treat VARA as a separate application with a
documented external interface. KiloNode should not emulate VARA internals.

## Planned Boundary

- VARA process is started and configured by the operator.
- KiloNode stores only host, port, mode, and enablement settings.
- Initial KiloNode support is read-only status and manual bench guidance.
- TX and live CONNECT remain blocked until separate safety milestones.

## Current Status

| Item | Status |
| ---- | ------ |
| Config schema | planned |
| Read-only status | planned |
| Receive bench notes | planned |
| TX bridge | blocked |
| Live CONNECT | blocked |

## Safety Notes

Do not connect PTT or enable transmit while using future adapter prototypes
unless a later lab-only TX milestone explicitly documents that path.
