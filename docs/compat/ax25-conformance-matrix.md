# AX.25 Conformance Matrix

| Area                           | Status      | Notes |
|--------------------------------|-------------|-------|
| Address encode/decode          | implemented | Destination, source, SSID, digipeater list, final bit, and repeated bit are covered by tests. |
| UI frame encode/decode         | implemented | UI frame body handling exists for RX and TX helpers. |
| PID handling                   | partial     | PID is decoded when expected and emitted for UI frames. Full PID matrix is planned. |
| Digipeater path decode         | partial     | Digipeater list and repeated bit are decoded. Command/response interpretation is deferred. |
| I frame control classification | implemented | Class, N(S), N(R), and P/F extraction are compile-tested for modulo 8 control bytes. |
| S frame control classification | implemented | RR, RNR, REJ, and SREJ names are compile-tested. |
| U frame control classification | implemented | SABM, SABME, DISC, DM, UA, FRMR, UI, XID, and TEST names are compile-tested. |
| SABM/SABME handling            | partial     | Isolated state core handles setup and returns send-UA intents. No live response is sent. |
| UA handling                    | partial     | Isolated state core completes setup/release waits. |
| DISC/DM handling               | partial     | Isolated state core tears down links and returns bounded action intents. |
| RR/RNR/REJ handling            | partial     | Isolated sequence accounting updates ack, busy, and retransmit-needed flags. |
| SABM frame build               | implemented | Raw AX.25 body builder emits SABM control frames without HDLC, FCS, KISS, or FX.25 wrapping. |
| SABME frame build              | implemented | Raw AX.25 body builder emits SABME control frames. |
| UA frame build                 | implemented | Raw AX.25 body builder emits UA response frames. |
| DM frame build                 | implemented | Raw AX.25 body builder emits DM response frames. |
| DISC frame build               | implemented | Raw AX.25 body builder emits DISC frames. |
| RR frame build                 | implemented | Raw AX.25 body builder emits RR frames with modulo 8 N(R). |
| RNR frame build                | implemented | Raw AX.25 body builder emits RNR frames with modulo 8 N(R). |
| REJ frame build                | implemented | Raw AX.25 body builder emits REJ frames with modulo 8 N(R). |
| Action-to-frame mapping        | implemented | Send action intents map to frame plans only. No TX queue writes or dispatch occur. |
| I-frame data build             | planned     | Connected information transfer frame generation is deferred. |
| Live TX queue integration      | planned     | Response frame queueing is not wired to runtime paths. |
| Connected-mode state machine   | partial     | First isolated unit-tested core exists. It is not wired to runtime RX/TX. |
| Timers                         | scaffold    | T1 and T3 are action intents only. No scheduler is started. |
| Retries                        | partial     | N2 retry count is used by isolated timeout transitions. |
| Modulo 8                       | implemented | Basic sequence increment, receive accounting, and ack handling are tested. |
| Modulo 128                     | planned     | Parameter placeholder exists. Extended control decode is deferred. |
| FRMR handling                  | partial     | Control subtype and protocol-error action placeholder exist. |
| TEST/XID handling              | scaffold    | Control subtype names exist. No procedures are implemented. |
| Live connected-mode sessions   | planned     | CONNECT, RF shell binding, BBS binding, and dispatch integration are not implemented. |
| FCS handling                   | deferred    | KISS-facing code operates at AX.25 body boundary. |
| KISS payload boundary          | implemented | Existing KISS encode/decode treats AX.25 bytes as payload. |
