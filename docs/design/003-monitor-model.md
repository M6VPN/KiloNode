# Monitor Model

The diagnostic monitor flow is:

```text
raw bytes -> KISS stream parser -> KISS frame -> AX.25 decode -> monitor line
```

The KISS stream parser owns its internal buffers. Completed frames are copied
into a caller-owned `struct kn_buffer` when popped. The popped frame metadata
points into that caller-owned buffer.

The AX.25 decoder borrows packet bytes from the caller. Monitor formatting must
complete before the caller releases the decoded packet buffer.

The default KISS stream frame limit is `KN_KISS_STREAM_DEFAULT_MAX_FRAME`.
Callers can provide a smaller or larger limit at parser initialization. If a
frame exceeds the configured limit, the parser discards bytes until the next
`FEND` and then resumes parsing.

Malformed frames are diagnostic events. They do not stop the parser permanently,
do not affect routing, and are formatted as clear monitor lines with length and
error code.

Monitor formatting is for diagnostics only. It is not routing logic, node
policy, BBS behavior, or protocol state.
