# Mercury OFDM Interface Questions

Answer these questions before implementing a Mercury adapter:

- Does Mercury expose TCP?
- Is the interface stream-oriented or packet-oriented?
- Does it carry raw bytes, frames, files, or messages?
- Does it preserve packet boundaries?
- Does it provide ARQ, or is KiloNode responsible?
- Does it expect AX.25, IP, custom frames, or application payload?
- How is link status queried?
- How is TX requested?
- How is RX delivered?
- Is PTT handled internally?
- Is there a test modem loopback mode?
- Are there sample frames or captures?
- What errors are reported for modem unavailable, link down, bad input, and TX refusal?
- What timing constraints apply to status, RX, and TX paths?

Until these are answered, Mercury remains a planned external modem profile only.
