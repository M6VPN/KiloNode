# ARDOP External Modem Roadmap

ARDOP is a planned external modem adapter target for KiloNode. It is not
implemented in the current release.

Future KiloNode support should use ARDOP's host interface as an external
process boundary. KiloNode should not embed ARDOP modem code.

## Planned Boundary

- ARDOP modem process is started and configured by the operator.
- KiloNode stores host, command port, data port, mode, and enablement settings
  only after a later config milestone defines them.
- Initial support is read-only status and manual bench guidance.
- TX and live CONNECT remain blocked until separate safety milestones.

## Current Status

| Item | Status |
| ---- | ------ |
| Config schema | planned |
| Read-only status | planned |
| Host interface adapter | planned |
| TX bridge | blocked |
| Live CONNECT | blocked |

## Safety Notes

Do not enable radio transmit, PTT, or live session behavior through ARDOP until
the response safety checklist and a lab-only modem milestone are complete.
