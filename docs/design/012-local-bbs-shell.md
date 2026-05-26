# Local BBS Shell

The local BBS shell exposes the KiloNode-native message store through the local
TCP node shell. It is for local testing and administration only. It does not
provide RF BBS access.

The normal node shell command `BBS` enters BBS mode when the `bbs` config block
is enabled and the message store opens successfully. BBS mode uses the prompt
`BBS>`. `EXIT` returns to the normal node shell prompt. `BYE` and `QUIT` close
the shell session.

Commands:

| Command | Result |
| ------- | ------ |
| `HELP` | Lists local BBS commands |
| `AREAS` | Lists bulletin areas found in stored bulletins |
| `LIST` | Lists non-deleted private messages and bulletins |
| `LIST PRIVATE` | Lists private messages |
| `LIST BULLETINS` | Lists bulletins |
| `LIST AREA <area>` | Lists bulletins in one area |
| `LIST TO <callsign>` | Lists private messages for one callsign |
| `LIST FROM <callsign>` | Lists messages from one callsign |
| `READ <id>` | Prints message metadata and body |
| `SEND PRIVATE <from> <to> <subject>` | Starts private message body entry |
| `SEND BULLETIN <from> <area> <subject>` | Starts bulletin body entry |
| `KILL <id>` | Soft-deletes a message |
| `EXIT` | Leaves BBS mode |
| `BYE` | Closes the shell session |
| `QUIT` | Closes the shell session |

`SEND` uses multiline body entry. Input continues until a line containing only
`.`. A line containing only `..` stores one literal dot line. The body size is
bounded by the configured message store limit.

Calls and bulletin areas use the existing message validation rules. Private
message destinations must be valid callsigns. Bulletin areas are normalized to
uppercase and must use the KiloNode area character set. Subjects are bounded by
the message model.

`READ` marks the message read through the global message read flag. Per-user
read state is deferred.

Output replaces unsafe control characters with `?` before writing them to the
terminal. Message bodies are never assumed to be NUL-terminated.

This is KiloNode-native behavior. It is not BPQ or LinBPQ command
compatibility.

Deferred work:

- RF BBS access
- user accounts
- authentication
- message forwarding
- FBB compatibility
- BPQ and LinBPQ message import and export
- area subscriptions
- message expiry and purge
- binary attachments
