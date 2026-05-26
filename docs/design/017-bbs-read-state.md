# BBS Read State

Read state is per BBS user. Message metadata remains the source of truth for
message content and deletion, while read markers live in per-user files.

Read-state layout:

```text
messages/
	read/
		M6VPN-1.read
		N0CALL.read
```

Each read-state file stores one message ID per line:

```text
1
7
23
```

Rules:

- new messages are unread for every user
- `READ <id>` marks that message read for the current BBS identity
- `MARKREAD <id>` marks a message read without printing the body
- `LIST` and list filters show `read=yes` or `read=no`
- `UNREAD` lists visible messages not marked read for the current user
- deleted messages stay hidden
- missing read-state files mean all visible messages are unread
- corrupt read-state files are rejected consistently

Supported filters keep using the message index layer:

- `LIST`
- `LIST PRIVATE`
- `LIST BULLETINS`
- `LIST AREA <area>`
- `LIST TO <callsign>`
- `LIST FROM <callsign>`

Deferred work:

- per-area subscriptions
- per-user forwarding preferences
- message expiry
- import and export
- BPQ/LinBPQ mailbox compatibility
- FBB forwarding state
