# M1 Known Limitations

M1 does not include:

- Live CONNECT.
- Real connected-mode AX.25 sessions.
- I-frame payload delivery to shell or BBS.
- Real response TX queueing.
- Real retransmission dispatch.
- NET/ROM routing.
- RF BBS access.
- BBS forwarding.
- FBB forwarding.
- LinBPQ/BPQ command compatibility.
- FX.25 FEC encode/decode.
- FX.25 TX wrapping.
- Real daemon scheduler dispatch.
- Channel-busy policy.
- PTT control.
- RF sysop authentication.
- Persistent AX.25 runtime diagnostics.
- Automatic promotion of manual captures into committed fixtures.

M1 does include diagnostic scaffolds for several of these areas. Those scaffolds
must not be described as full protocol support.
