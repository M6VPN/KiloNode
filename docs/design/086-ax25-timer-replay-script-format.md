# AX.25 Timer Replay Script Format

Timer replay scripts are line-oriented text files. Comments start with `#`.
Each non-comment line contains one command.

Required header fields:

| Command | Purpose |
|---------|---------|
| `name <name>` | Replay name for diagnostics. |
| `node <callsign>` | Local node callsign. |
| `remote <callsign>` | Remote station callsign. |
| `port <name>` | Diagnostic port key. |

Supported commands:

| Command | Purpose |
|---------|---------|
| `now <ms>` | Set injected monotonic time. |
| `advance <ms>` | Advance injected monotonic time. |
| `params modulo=8 window=<n> t1=<ms> t2=<ms> t3=<ms> n2=<n>` | Set AX.25 params. |
| `event local-connect` | Apply local connect test event. |
| `event local-disconnect` | Apply local disconnect test event. |
| `event rx-sabm` | Apply received SABM. |
| `event rx-sabme` | Apply received SABME. |
| `event rx-ua` | Apply received UA. |
| `event rx-disc` | Apply received DISC. |
| `event rx-dm` | Apply received DM. |
| `event rx-rr nr=<n>` | Apply received RR. |
| `event rx-rnr nr=<n>` | Apply received RNR. |
| `event rx-rej nr=<n>` | Apply received REJ. |
| `event rx-i ns=<n> nr=<n> len=<bytes>` | Apply received I-frame summary. |
| `process-expired` | Collect and process expired logical timers. |
| `expect ...` | Check an immediate expectation. |

Supported expectations:

| Expectation | Meaning |
|-------------|---------|
| `expect state=<state>` | Check current connection state. |
| `expect retry=<n>` | Check AX.25 connection retry count. |
| `expect timer-running T1 true` | Check logical timer running state. |
| `expect timer-expired T1 true` | Check logical timer expiry at current time. |
| `expect action <action-name>` | Check last action list. |
| `expect plan <frame-kind>` | Check last frame-plan list. |
| `expect counter <name> <value>` | Check scheduler/runtime counters. |
| `expect tx-writes <value>` | Check TX write attempts. |
| `expect connection-count <value>` | Check diagnostic table size. |
| `expect last-error <code>` | Check last replay status label. |

Replay files are hostile input. Unknown commands and unknown expectation keys
are rejected. Replay files cannot execute shell commands, open paths, or open
devices.
