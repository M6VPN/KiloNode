# AX.25 Loopback Safety Boundary

The loopback simulator is an offline diagnostic tool. It proves internal AX.25
state transitions and frame preparation without crossing into live transmit
paths.

Safety counters in reports include:

- `real_tx_queue_writes`
- `dispatch_calls`
- `prepared_frames_generated`
- `raw_ax25_frames_transferred`
- `fx25_frames_generated`
- endpoint states
- mismatch count

`real_tx_queue_writes`, `dispatch_calls`, and `fx25_frames_generated` must stay
zero for committed fixtures.

## Not Exposed

M2.1 does not add:

- local shell CONNECT
- BBS CONNECT
- RF command CONNECT
- control socket CONNECT
- prepared-to-real-TX promotion
- TX dispatch
- RF BBS access
- NET/ROM routing
- BBS or FBB forwarding

The `kilonode-compat` commands only parse and run `.loop` fixture files. They do
not open hardware, serial ports, TCP KISS sources, or daemon sockets.

## Prepared Frames

Prepared frames remain diagnostics only. The loopback link copies prepared raw
AX.25 bytes into the peer endpoint input path. This is an in-memory simulator
operation, not a TX queue write.

The prepared-to-TX bridge gate from M1.47 remains disabled in runtime.
