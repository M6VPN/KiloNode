# Store Locking

The filesystem BBS store uses a local advisory lock to reduce unsafe concurrent
writes from tools and the daemon.

The lock file is:

```text
.kilonode-store.lock
```

It lives below the store root. The current implementation uses a POSIX
`fcntl()` write lock. It is advisory, so cooperating KiloNode tools and daemon
code must take the lock before writing.

Writer operations that take or use an exclusive lock include:

- message create
- message soft delete
- global message mark-read compatibility path
- user create and update
- read-state save
- reindex through maintenance tools
- repair
- purge deleted
- export snapshot

Readers usually run without a lock. Export takes an exclusive lock because it
needs a stable local snapshot.

The lock is released when the lock object is released or the process exits. It
does not use stale PID-file deletion. Parent directories are not created by the
lock layer.

Limitations:

- this is local advisory locking, not a distributed lock
- network filesystems may have different lock behavior
- there is no shared read lock in this pass
- recovery from non-cooperating writers is out of scope
