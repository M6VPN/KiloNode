# Store Maintenance

KiloNode store maintenance checks the native filesystem BBS store before later
RF access and forwarding layers depend on it. The maintenance code treats all
metadata, bodies, indexes, user files, read-state files, and export paths as
untrusted input.

The `kilonode-store` tool provides:

```text
check
repair
reindex
stats
purge-deleted
export DEST
```

Check findings use three severities:

- `info` for changes made by repair
- `warning` for recoverable inconsistencies
- `error` for integrity problems that should block normal operation

The checker validates the store root, managed directories, `next-id`, message
metadata, body files, required indexes, user files, and read-state files. It
reports corrupt metadata instead of guessing missing fields.

Repair is intentionally conservative. It may:

- create missing managed directories
- create or raise `next-id`
- rebuild indexes
- deduplicate read-state files

Repair does not invent message metadata, rewrite corrupt messages, renumber
message IDs, or delete active messages.

`purge-deleted` physically removes messages already marked deleted. It removes
the matching metadata and body files, rebuilds indexes, and does not reuse
message IDs. ID compaction is intentionally unsupported because message IDs may
be referenced externally later.

Export creates a KiloNode-native directory snapshot with `manifest.txt` plus
copies of `meta/`, `msg/`, `index/`, `users/`, and `read/`. The destination must
not be inside the source store and must be empty. Symlinks are not followed.

Store stats include total messages, private messages, bulletins, deleted
messages, users, read-state files, bulletin areas, body bytes, newest message
ID, and next ID.

This is KiloNode-native maintenance. It is not BPQ or LinBPQ mailbox
compatibility.

Deferred work:

- import
- BPQ/LinBPQ import and export
- forwarding queues
- message expiry policy
- archive compression
- SQLite or other backend migration
