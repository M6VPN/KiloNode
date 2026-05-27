# LinBPQ Node Risk Register

This register tracks compatibility planning risks. It does not authorize source
inspection or implementation work.

| ID | Severity | Status | Area | Risk | Mitigation |
| -- | -------- | ------ | ---- | ---- | ---------- |
| GPL-CLEANROOM | critical | open | clean-room | GPL contamination risk | Use black-box observations only |
| OUTPUT-COPY | high | open | behaviour | Prompt/output copying risk | Store observations as data, not code |
| COMMAND-AMBIGUITY | medium | open | parser | Command ambiguity risk | Require explicit observation coverage |
| AX25-CONNECTED | high | deferred | protocol | Connected-mode AX.25 prerequisite | Keep CONNECT blocked |
| NETROM | high | deferred | routing | NET/ROM prerequisite | Keep NODES and ROUTES blocked |
| BBS-MAILBOX | high | deferred | bbs | BBS mailbox format unknown | Capture mailbox behaviour later |
| FORWARDING | high | deferred | forwarding | Forwarding protocol unknown | Capture protocol boundary later |
| TX-SAFETY | critical | open | transmit | Unsafe transmit risk | Keep dispatch gated and local-admin only |
| SYSOP-AUTH | high | deferred | auth | User/sysop authentication risk | Design auth before sysop commands |
| NATIVE-REGRESSION | medium | open | native | Regression against KiloNode-native shell | Keep native tests separate |

Run:

```text
./build/kilonode-compat risk-report tests/fixtures/compat/linbpq-node/requirements.plan
```
