# AX.25 Loopback Retransmission Buffer

M2.5 adds a bounded retransmission buffer to AX.25 loopback endpoints. The
buffer is diagnostic state owned by the simulator. It records raw AX.25 I-frame
bytes that were already built for in-memory endpoint exchange.

Each record stores:

- diagnostic ID
- N(S)
- N(R)
- payload length
- segment index
- bounded raw AX.25 bytes
- retry and acknowledgement status
- replay count

RR frames mark buffered records acknowledged according to modulo-8 N(R). REJ
diagnostics mark matching N(S) records as retry-needed. The `replay-buffer`
loopback script command copies buffered raw AX.25 bytes back through the
in-memory link only.

## Safety Boundary

The retransmission buffer is not the real TX queue. It does not call TX queue
APIs, KISS transports, dispatch, daemon CONNECT, shell, BBS, RF BBS, NET/ROM,
or FX.25 code.

The following remain blocked:

- live CONNECT
- live retransmission dispatch
- real TX queue writes
- automatic dispatch
- shell or BBS payload binding
- FX.25 wrapping or FEC

## Script Surface

M2.5 adds these loopback script commands and expectations:

- `event A|B send-rej nr=<n>`
- `replay-buffer A|B max-frames=<n>`
- `expect A|B retransmit-buffer=<n>`
- `expect A|B retransmit-needed=<n>`
- `expect A|B retransmit-acked=<n>`
- `expect A|B retransmit-replayed=<n>`
- `expect A|B retransmit-full=<n>`

All counters are deterministic diagnostics. A passing fixture must still show
`tx-writes 0`, `dispatch-calls 0`, and `fx25-frames 0`.
