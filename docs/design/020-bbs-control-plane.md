# BBS Control Plane

This pass exposes read-only BBS store state through the local daemon control
socket. The protocol is KiloNode-native and line-oriented. It is not a BPQ or
LinBPQ command protocol.

## Scope

The control socket can report BBS status, store counters, bulletin areas, users,
message summaries, and a bounded message preview. It does not modify store data.

Supported control commands:

| Command | Result |
| ------- | ------ |
| `BBS STATUS` | BBS enabled, open, and store path state |
| `BBS STATS` | Message, user, area, body byte, and ID counters |
| `BBS AREAS` | Bulletin areas with message counts and newest IDs |
| `BBS USERS` | Local BBS users with disabled, sysop, login, and seen state |
| `BBS MESSAGES` | Visible non-deleted message summaries |
| `BBS MESSAGES PRIVATE` | Private message summaries |
| `BBS MESSAGES BULLETINS` | Bulletin summaries |
| `BBS MESSAGES AREA <area>` | Bulletin summaries for one area |
| `BBS MESSAGES TO <callsign>` | Private messages for one recipient |
| `BBS MESSAGES FROM <callsign>` | Messages from one sender |
| `BBS MESSAGE <id>` | One message summary plus bounded body preview |

`kilonodectl` maps local commands to these protocol lines:

| CLI command | Control line |
| ----------- | ------------ |
| `bbs status` | `BBS STATUS` |
| `bbs stats` | `BBS STATS` |
| `bbs areas` | `BBS AREAS` |
| `bbs users` | `BBS USERS` |
| `bbs messages` | `BBS MESSAGES` |
| `bbs messages --private` | `BBS MESSAGES PRIVATE` |
| `bbs messages --bulletins` | `BBS MESSAGES BULLETINS` |
| `bbs messages --area AREA` | `BBS MESSAGES AREA AREA` |
| `bbs messages --to CALLSIGN` | `BBS MESSAGES TO CALLSIGN` |
| `bbs messages --from CALLSIGN` | `BBS MESSAGES FROM CALLSIGN` |
| `bbs message ID` | `BBS MESSAGE ID` |

## Output

Responses use one or more lines. Multi-line responses end with `END`.

Example:

```text
OK BBS MESSAGES count=2 truncated=false
BBS MSG id=1 type=private from=M6VPN-1 to=N0CALL created=1710000000 deleted=false subject="Test private"
BBS MSG id=2 type=bulletin from=M6VPN-1 area=GENERAL created=1710000100 deleted=false subject="Test bulletin"
END
```

List responses return at most 32 rows in this pass. If more rows exist, the
first line includes `truncated=true`. Message body previews are limited to 200
bytes. Printable bodies are returned as quoted text. Binary bodies are returned
as a hex preview with the original byte count.

## Safety Rules

The control plane treats store data as hostile. Area and callsign filters are
validated before use. Subjects, paths, and body previews are escaped or replaced
before formatting. Deleted messages are hidden from list commands and return a
deterministic error for direct message lookups.

The daemon does not run store repair, purge, export, message deletion, or user
updates through the control socket in this pass.

## Deferred Work

- Authenticated write control
- Remote sysop commands
- Repair, purge, export, and delete over the control socket
- RF BBS access
- Forwarding status
- BPQ and LinBPQ mailbox compatibility
- FBB forwarding state
