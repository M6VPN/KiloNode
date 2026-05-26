# LinBPQ Compatibility Lab

Later compatibility work may use local LinBPQ black-box inputs:

- `~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/linbpq`
- `~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/bpq32.cfg.example`

They are not used in the local BBS shell pass.

Future compatibility work should use externally visible behavior only:

- black-box node sessions
- packet captures
- mailbox file observations
- config examples
- externally visible forwarding behavior

GPL source code must not be copied, translated, or structurally mirrored. BPQ
and LinBPQ command compatibility is a future compatibility layer, not the
KiloNode-native shell design.
