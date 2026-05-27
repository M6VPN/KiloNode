# LinBPQ Node Command Profiles

Profiles describe command shape only. They do not include copied prompts,
output text, parser logic, command tables, or implementation algorithms.

| Command | Category | Transport | Args | Reply | Stateful | Connected Mode | Status |
| ------- | -------- | --------- | ---- | ----- | -------- | -------------- | ------ |
| HELP | informational | telnet | none | one-or-more-lines | false | false | needs-observation |
| INFO | informational | telnet | none | one-or-more-lines | false | false | needs-observation |
| PORTS | informational | telnet | none | one-or-more-lines | false | false | needs-observation |
| USERS | informational | telnet | none | one-or-more-lines | false | false | needs-observation |
| UNKNOWN | unknown-handling | telnet | free-text | one-line | false | false | needs-observation |
| BBS | bbs | connected-ax25 | optional | session-transition | true | true | blocked |
| CONNECT | session | connected-ax25 | required | session-transition | true | true | blocked |
| NODES | routing | netrom | none | one-or-more-lines | true | true | blocked |
| ROUTES | routing | netrom | none | one-or-more-lines | true | true | blocked |

The source fixture is
`tests/fixtures/compat/linbpq-node/command-profiles.plan`.
