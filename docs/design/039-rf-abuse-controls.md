# RF Abuse Controls

M1.25 adds KiloNode-native abuse controls for RF command ingress. The controls
only harden inbound UI command handling. They do not add new RF user features,
authentication, connected-mode AX.25, NET/ROM, RF BBS access, or compatibility
commands.

Each observed RF command source is keyed by normalized callsign plus SSID. The
runtime table tracks command window counts, accepted commands, rejected
commands, reply counts, last-seen time, the last reject reason, and temporary
ignore state. The table is bounded and evicts the oldest source by last-seen
time when full.

Command rate limiting is controlled by:

```text
rate-limit-enabled true
rate-limit-commands 6
rate-limit-window-seconds 60
```

Unknown, malformed, overlong, and rejected commands count toward the source's
diagnostic counters. A rate-limited command is recorded as an RF command event
with status `rate-limited` and does not queue a reply.

Reply suppression is separate from command acceptance:

```text
reply-rate-limit-commands 3
reply-rate-limit-window-seconds 60
```

When the reply limit is exceeded, the command can still be accepted, but the
event records `reply-suppressed` and no TX frame is queued.

Auto-ignore is disabled by default:

```text
auto-ignore-enabled false
auto-ignore-after-rejects 10
auto-ignore-seconds 900
```

When enabled, repeated rejects within the command rate window temporarily ignore
the source. Expired temporary ignores are cleared on the next check. Auto-ignore
is runtime-only in this pass.

Read-only diagnostics:

```text
RF ABUSE STATUS
RF ABUSE SOURCES
RF ABUSE SOURCE <callsign>
```

`kilonodectl` mappings:

```text
kilonodectl --socket /tmp/kilonode/control.sock rf abuse-status
kilonodectl --socket /tmp/kilonode/control.sock rf abuse-sources
kilonodectl --socket /tmp/kilonode/control.sock rf abuse-source N0CALL
```

This is not authentication, authorization, or BPQ/LinBPQ access-control
compatibility.

Deferred work:

- Persistent auto-ignore entries
- Control add/remove ignore commands
- Per-port RF policy
- Per-command RF policy
- Authenticated RF sysop controls
- BPQ/LinBPQ-compatible access controls
