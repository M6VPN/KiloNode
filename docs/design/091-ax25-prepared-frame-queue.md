# AX.25 Prepared Frame Queue

The prepared-frame queue stores generated AX.25 response diagnostics after the action mapper creates frame plans. It is not the RF TX queue and it has no dispatch path.

Each prepared record has a runtime-local ID, injected creation time, connection ID, port, local and remote callsigns, optional digipeater path, action intent, frame kind, P/F bit, sequence fields, status, reason, and bounded raw AX.25 bytes when raw building is enabled.

Raw bytes are AX.25 body bytes only. They do not include HDLC flags, FCS, KISS framing, or FX.25 wrapping. Payload storage is bounded, and diagnostics expose only short summaries and bounded hex previews.

The queue is bounded and rejects new records when full. It does not evict old records in this pass, so overflow is visible through counters.

No prepared-frame API writes to the real TX queue.
