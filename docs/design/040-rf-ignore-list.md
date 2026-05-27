# RF Ignore List

M1.25 adds a simple manual RF ignore list for command ingress. It is
KiloNode-native and only affects RF command parsing and reply queueing.

The configured file path is optional:

```text
ignore-list-path ./var/rf-ignore.txt
```

The file format is plain text:

```text
# comments begin with a hash
N0CALL manual
BAD1-7 noisy test source
```

Rules:

- Blank lines are ignored.
- Lines beginning with `#` are ignored.
- The first token is a callsign, normalized through the KiloNode callsign
  parser.
- Text after the first token is a bounded reason.
- Duplicate callsigns update the reason.
- Invalid lines reject the file load.
- Manual ignores do not expire.

Ignored sources are rejected before reply generation. The RF command event
records status `ignored`, and no TX frame is queued.

Read-only diagnostics:

```text
RF IGNORE LIST
```

`kilonodectl` mapping:

```text
kilonodectl --socket /tmp/kilonode/control.sock rf ignore-list
```

Temporary auto-ignore entries come from the runtime abuse table and are not
written to the manual ignore file in this pass.

Deferred work:

- Control add/remove ignore entries
- Persistent auto-ignore
- Ignore-file repair tooling
- Per-port ignore policy
- Per-command ignore policy
- Authenticated RF sysop controls
- BPQ/LinBPQ-compatible access controls
