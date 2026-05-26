# Local BBS Shell

The local BBS shell exposes the KiloNode-native message store through the local
TCP node shell. It is for local testing and administration only. It does not
provide RF BBS access.

The normal node shell command `BBS <callsign>` enters BBS mode when the `bbs`
config block is enabled and the message store opens successfully. The callsign
is validated and used as the local BBS identity. BBS mode uses the prompt
`BBS <callsign>>`. `EXIT` returns to the normal node shell prompt. `BYE` and
`QUIT` close the shell session.

Commands:

| Command | Result |
| ------- | ------ |
| `HELP` | Lists local BBS commands |
| `WHOAMI` | Shows the current BBS callsign identity |
| `USERS` | Lists local BBS users |
| `AREAS` | Lists bulletin areas found in stored bulletins |
| `LIST` | Lists non-deleted private messages and bulletins |
| `LIST PRIVATE` | Lists private messages |
| `LIST BULLETINS` | Lists bulletins |
| `LIST AREA <area>` | Lists bulletins in one area |
| `LIST TO <callsign>` | Lists private messages for one callsign |
| `LIST FROM <callsign>` | Lists messages from one callsign |
| `UNREAD` | Lists messages unread by the current identity |
| `READ <id>` | Prints message metadata and body |
| `MARKREAD <id>` | Marks one message read without printing the body |
| `SEND PRIVATE <to> <subject>` | Starts private message body entry |
| `SEND BULLETIN <area> <subject>` | Starts bulletin body entry |
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

`READ` marks the message read for the current BBS identity only. `LIST` output
shows the current user's read marker as `read=yes` or `read=no`.

Output replaces unsafe control characters with `?` before writing them to the
terminal. Message bodies are never assumed to be NUL-terminated.

This is KiloNode-native behavior. It is not BPQ or LinBPQ command
compatibility.

Deferred work:

- RF BBS access
- authentication
- message forwarding
- FBB compatibility
- BPQ and LinBPQ message import and export
- area subscriptions
- message expiry and purge
- binary attachments
