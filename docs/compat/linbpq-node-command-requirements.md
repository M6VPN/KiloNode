# LinBPQ Node Command Requirements

This table is a planning artifact. It does not claim BPQ/LinBPQ compatibility.

| Command | Status | Priority | Observation | Notes |
| ------- | ------ | -------- | ----------- | ----- |
| HELP | needs-observation | high | synthetic | Manual black-box observations needed |
| INFO | needs-observation | high | synthetic | Manual black-box observations needed |
| PORTS | needs-observation | high | synthetic | Manual black-box observations needed |
| USERS | needs-observation | medium | synthetic | Manual black-box observations needed |
| HEARD | implemented-native | high | planned | KiloNode-native behaviour exists, compatibility deferred |
| STATS | implemented-native | medium | planned | KiloNode-native behaviour exists, compatibility deferred |
| BYE | planned | medium | planned | Session handling deferred |
| UNKNOWN | needs-observation | high | synthetic | Error-path baseline only |
| BBS | blocked | high | planned | RF BBS access deferred |
| CONNECT | blocked | critical | planned | Connected-mode AX.25 deferred |
| NODES | blocked | high | planned | NET/ROM deferred |
| ROUTES | blocked | high | planned | NET/ROM deferred |

The source fixture is
`tests/fixtures/compat/linbpq-node/requirements.plan`.
