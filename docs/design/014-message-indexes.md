# Message Indexes

Message indexes let LIST, AREAS, READ, and later forwarding code find messages
without repeatedly scanning raw message files. The indexes are derived data. The
message metadata and body files remain the source of truth.

Store layout:

```text
messages/
	meta/
		next-id
	msg/
		00000001.meta
		00000001.body
	index/
		all.idx
		private.idx
		bulletin.idx
		area-GENERAL.idx
		to-N0CALL.idx
		from-M6VPN-1.idx
```

Each index line is KiloNode-native:

```text
id|type|from|to-or-area|created|read|deleted|subject
```

Index files are bounded line-oriented text. Corrupt index files are rebuilt from
message metadata. Missing indexes are rebuilt. Deleted messages are hidden from
rebuilt indexes and from indexed list calls.

Rebuild policy:

- store open creates `index/` if needed
- missing required indexes trigger rebuild
- corrupt required indexes trigger rebuild
- manual `kilonode-msg reindex` rebuilds all indexes
- corrupt message metadata is skipped during rebuild
- missing or invalid bodies cause that message to be skipped during rebuild

Soft delete sets the metadata deleted flag and rebuilds indexes. Per-user read
state is stored separately under `read/`, so one user's READ command does not
change the message metadata for other users.

Limits:

- message body limit follows the store configuration
- index lines are capped internally
- index list APIs use caller-provided arrays
- bulletin area names use the message area limit
- rebuild currently supports up to 4096 allocated message IDs

This is KiloNode-native indexing. It is not BPQ or LinBPQ mailbox
compatibility.

Deferred work:

- expiry
- compaction
- forwarding queues
- import and export
- database backend
