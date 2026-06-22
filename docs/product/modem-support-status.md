# Modem Support Status

| Modem/path | Status | KiloNode interface | TX | CONNECT | Notes |
| ---------- | ------ | ------------------ | -- | ------- | ----- |
| Dire Wolf TCP KISS | receive-only usable | KISS TCP | blocked | blocked | first practical hobbyist path |
| KiloTNC KISS | receive path documented | KISS serial/TCP | blocked | blocked | manual hardware pending |
| Mercury OFDM | planned/discovery | external modem boundary | blocked | blocked | interface details needed |
| VARA HF/FM | planned | external modem boundary | blocked | blocked | future adapter |
| ARDOP | planned | external modem boundary | blocked | blocked | future adapter |

External modem status commands are read-only in this pass. They do not start
processes, open modem sockets, enable CONNECT, or enable transmit.
