# RF Reply Policy

RF replies are optional and disabled by default. When enabled, replies are
queued as AX.25 UI frames through the existing TX queue. The RF command layer
does not write to transports and does not dispatch queued frames.

Reply queueing requires:

- `rf-command reply-enabled true`
- The source is not ignored or command-rate-limited
- The source has not exceeded the reply rate limit
- TX policy enabled
- UI enqueue allowed by TX policy
- The inbound port is enabled and open
- For non-dry-run queueing, the M1.23 real KISS gates pass
- For non-dry-run queueing, the target port has `tx-enabled true`

If a reply cannot be queued, the command event records `reply_queued=false` and
a deterministic reason. Normal configs keep RF command parsing and replies
disabled.

When reply rate limiting suppresses a reply, the command event records
`reply-suppressed`. When TX gates block queueing, the command event records
`tx-gate-blocked`.

Replies are single-line, bounded by `max-reply-bytes`, and use safe summaries.
Reverse digipeater paths are deferred.

Queued replies remain in the TX queue until an explicit dispatch command runs.
There is no automatic dispatch from RF command ingress.

Deferred work:

- Connected-mode node sessions
- RF BBS access
- BPQ/LinBPQ command compatibility
- NET/ROM
- Forwarding
- Authentication and sysop commands
- Channel busy and PTT policy
