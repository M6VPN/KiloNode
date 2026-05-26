# Frame Model

KISS ends at byte escaping and transport framing. After a KISS payload is
unescaped, the remaining bytes are treated as an AX.25 frame body without HDLC
flags or FCS.

The AX.25 frame model stores:

- Destination address
- Source address
- Zero or more digipeater addresses
- Control byte
- PID byte when present
- Payload pointer and payload length

Decoded frames do not own packet bytes. The payload pointer references the
caller-provided input buffer, so the caller must keep that buffer alive while
using the decoded frame. Encoded frames write into `struct kn_buffer`, which owns
its allocated storage.

All frame data uses explicit lengths. Packet data is binary, can contain NUL
bytes, and must never be treated as a C string.

The current compile-time digipeater limit is `KN_AX25_MAX_DIGIS`. The project
does not set a final maximum packet size in this pass, but transport and node
layers should enforce practical limits before accepting untrusted input.

These primitives are intended for later KISS adapters, monitor output, node
routing decisions, and BBS message handling. This pass does not implement those
higher layers.
