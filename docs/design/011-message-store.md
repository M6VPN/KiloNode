# Message Store

The message store is a local filesystem-backed storage layer for future BBS
work. It stores private messages and bulletins. It is not exposed over RF in
this pass.

Message model:

- numeric message ID
- type: `private` or `bulletin`
- from callsign
- private destination callsign or bulletin area
- subject
- created timestamp
- updated timestamp
- read flag
- deleted flag
- body length

Message bodies are stored separately from metadata. Body bytes are treated as
opaque data and are not assumed to be NUL-terminated.

Filesystem layout:

```text
messages/
	meta/
		next-id
	msg/
		00000001.meta
		00000001.body
		00000002.meta
		00000002.body
```

Metadata format:

```text
id 1
type private
from M6VPN-1
to N0CALL
subject Test private
created 0
updated 0
read 0
deleted 0
body-length 20
```

Bulletins use `type bulletin` and store the bulletin area in the `to` field.
The format is KiloNode-native and is not BPQ or LinBPQ compatible.

IDs start at 1. `meta/next-id` stores the next ID to allocate. IDs are
monotonic and are not reused after delete. Corrupt next-ID metadata prevents the
store from opening.

Delete is soft delete. The metadata `deleted` flag is set to `1`. Normal listing
hides deleted messages. Direct reads of deleted messages return a deterministic
deleted error. Hard purge is deferred.

Limits:

- subject: 120 bytes
- callsign: existing KiloNode callsign formatter limit
- bulletin area: 32 bytes
- body: 65536 bytes by default

Corruption handling:

- corrupt next-ID metadata rejects store open
- corrupt message metadata is skipped while listing
- direct read of corrupt metadata returns a corrupt error
- missing body for existing metadata returns a corrupt error
- overlarge bodies are rejected

Deferred work:

- BBS user sessions
- message forwarding
- FBB compatibility
- import and export tools
- persistence compaction
- user authentication
- RF access
