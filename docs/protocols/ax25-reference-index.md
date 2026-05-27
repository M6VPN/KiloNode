# AX.25 Reference Index

Primary reference:

| File         | Standing  | Scope used here |
|--------------|-----------|-----------------|
| AX25.2.2.pdf | Normative | AX.25 frame layout, address fields, control fields, PID, I/S/U frame classes, timers, retry parameters, modulo modes, and connected-mode planning. |

KiloNode already implements:

| Area                   | Status      | Notes |
|------------------------|-------------|-------|
| Address fields         | Implemented | Destination, source, and digipeater address encode/decode are present. |
| UI frames              | Implemented | UI encode/decode and KISS wrapping are present. |
| PID handling           | Partial     | PID is decoded when the control field indicates it and used for UI text payloads. |
| Digipeater path bits   | Partial     | Repeated bit is preserved. Command/response use is deferred. |
| KISS payload boundary  | Implemented | AX.25 body bytes are carried inside KISS frames without HDLC flags or FCS. |

Planned from the reference:

| Area                 | Plan |
|----------------------|------|
| Control fields       | Add classification helpers for I, S, U, and UI frames. |
| I frames             | Classify now, implement connected transfer later. |
| Supervisory frames   | Classify RR, RNR, REJ, and SREJ now. State behaviour is deferred. |
| Unnumbered frames    | Classify SABM, SABME, DISC, DM, UA, FRMR, UI, XID, and TEST now. |
| Timers               | Add disabled parameter scaffolding for T1, T2, and T3. |
| Retries              | Add disabled N2 parameter scaffolding. |
| Modulo modes         | Add modulo 8 and modulo 128 parameter scaffolding. |
| Segmenting           | Defer until connected-mode design is active. |

Deliberately deferred:

| Area                         | Reason |
|------------------------------|--------|
| Connected-mode state machine | M1.32 only adds compile-tested structs and classification helpers. |
| Live CONNECT behaviour       | Shell and RF user access remains disabled. |
| Response frame generation    | No SABM/UA/DISC/RR/RNR/REJ transmit logic is added in this pass. |
| FCS handling                 | KISS-facing code continues to operate at the AX.25 body boundary. |
| NET/ROM and BBS binding      | Separate future milestones. |
