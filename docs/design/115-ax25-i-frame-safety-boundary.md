# AX.25 I-Frame Safety Boundary

M2.2 I-frame support is simulator and unit-test tooling only.

It does not add:

- live CONNECT
- shell or BBS connected-mode payload binding
- RF command ingress for CONNECT
- real TX queue writes
- TX dispatch
- retransmission dispatch
- RF BBS access
- NET/ROM routing
- BBS or FBB forwarding

The loopback simulator exchanges raw AX.25 bytes in memory. Prepared control
frames and I frames are diagnostics data for tests. They are not promoted to the
real TX queue.

Reports and fixtures assert:

```text
tx_writes=0
dispatch=0
fx25=0
```

Payload delivery records are bounded diagnostics. They are not a user session,
mailbox input, command input, or shell stream.

M2.3 segmented payload reassembly records follow the same boundary. They are
diagnostics only and are not bound to any live service.
