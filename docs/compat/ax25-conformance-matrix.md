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
| Connection key model           | implemented | Bounded port, local callsign, remote callsign, and digipeater path key with deterministic formatting. |
| Connection table scaffold      | implemented | Fixed-size in-memory table stores state, counters, actions, and frame plans. |
| AX.25 runtime scaffold         | implemented | Daemon-owned runtime exists with disabled defaults and diagnostic counters. |
| Read-only connection diagnostics | implemented | Control protocol and `kilonodectl` expose status, params, lists, details, and counters. |
| Control-plane connection listing | implemented | `AX25 CONNECTIONS` and port-filtered listing are bounded and read-only. |
| Control-plane params/counters  | implemented | `AX25 PARAMS` and `AX25 COUNTERS` report runtime snapshots only. |
| Live RX diagnostics feed       | implemented | Decoded inbound AX.25 connected-mode frames feed the diagnostic runtime only when explicitly enabled. |
| Inbound SABM diagnostic table update | implemented | Live feed can create diagnostic records from SABM when creation is explicitly enabled. |
| Inbound DISC/UA/DM/RR/RNR/REJ diagnostic update | implemented | Live feed updates existing diagnostic records and counters. |
| Inbound I-frame diagnostic update | implemented | Live feed updates sequence diagnostics and stores payload length only. |
| Live frame-plan retention      | implemented | Generated frame plans are retained in diagnostics only and are never queued. |
| Receive-only bench docs       | implemented | USB sound card, Dire Wolf, KiloTNC, serial, TCP, PTY, and Unix KISS receive workflows are documented. |
| Live RX diagnostics bench checklist | implemented | Bench checklist covers config check, TX gates, RX events, heard list, live counters, and connection diagnostics. |
| Synthetic bench KISS UI fixture | implemented | Receive-only KISS UI fixtures cover CQ and node-directed PING payloads. |
| Synthetic bench KISS SABM fixture | implemented | KISS SABM fixture validates setup-frame decode expectations. |
| Synthetic bench KISS DISC/RR fixtures | implemented | KISS DISC and RR fixtures validate disconnect and supervisory decode expectations. |
| Raw AX.25 bench fixtures      | implemented | AXIP-format packet-boundary fixtures carry raw AX.25 UI and SABM bytes. |
| Offline diagnostic replay from KISS captures | implemented | Bench KISS captures replay into the AX.25 diagnostics runtime with zero TX writes. |
| Offline diagnostic replay from raw AX.25 captures | implemented | Raw AX.25 bench captures replay through the same diagnostics harness. |
| SABM bench replay             | implemented | SABM fixtures create diagnostic connection records and retain response frame plans only. |
| DISC/RR bench replay          | partial     | Single-frame DISC/RR fixtures decode and are ignored without an existing connection. Multi-frame sequence fixtures are planned placeholders. |
| UI ignored by connected diagnostics | implemented | UI captures increment ignored UI counters and do not create connected-mode records. |
| TX writes zero assertion      | implemented | Diagnostic replay reports and expected checks require TX write attempts to remain zero. |
| Manual capture workspace      | implemented | User-provided receive-only captures can be kept outside committed fixtures. |
| Manual capture import         | implemented | `.capture` files import into a bounded workspace index without opening transports. |
| Manual AX.25 diagnostic replay | implemented | Imported captures replay through the offline AX.25 diagnostics harness with zero TX writes. |
| Manual connected-mode capture replay | planned | Multi-frame manual connected-mode replay remains future work beyond single capture files. |
| Real AX.25 source validation   | planned     | Manual bench validation with receive-only hardware is planned. CI remains synthetic. |
| Manual bench captures          | planned     | Future imported manual captures require review before becoming committed fixtures. |
| Inbound SABM table handling    | implemented | Unit tests create a table record and retain a UA frame plan. No response is queued. |
| Inbound DISC table handling    | implemented | Unit tests update an existing record and retain a UA frame plan. No response is queued. |
| Local-connect unit-test event  | implemented | Internal test helper drives the state core and produces a SABM frame plan. It is not user exposed. |
| Action-to-frame-plan retention | implemented | Last generated frame plans remain in record diagnostics only. |
| I-frame data build             | planned     | Connected information transfer frame generation is deferred. |
| Live TX queue integration      | planned     | Response frame queueing is not wired to runtime paths. |
| Connected-mode state machine   | partial     | First isolated unit-tested core exists. It is not wired to runtime RX/TX. |
| T1 logical timer scaffold      | implemented | Logical T1 uses injected monotonic millisecond values and maps to `timeout-t1`. |
| T2 logical timer scaffold      | scaffold    | Logical T2 can expire in the queue but delayed-ACK behaviour remains planned. |
| T3 logical timer scaffold      | implemented | Logical T3 uses injected monotonic millisecond values and maps to `timeout-t3`. |
| N2 retry accounting            | implemented | Bounded retry helper covers reset, increment, under-limit, and exhaustion checks. |
| Timer expiry event mapping     | partial     | T1 and T3 map to state-machine timeout events. T2 remains a planned placeholder. |
| Scheduler scaffold             | implemented | Offline scheduler applies timer/retry action intents and processes injected expiries. |
| Offline timer-driven diagnostics | implemented | Replay scripts drive synthetic events, injected time, scheduler expiry, state transitions, and expectations. |
| T1 timeout replay              | implemented | Fixtures cover connect retry, disconnect retry, and connection exhaustion diagnostics. |
| T1 retry exhaustion replay     | implemented | Fixture reaches N2 exhaustion and verifies disconnected/error diagnostics. |
| T3 timeout replay              | implemented | Fixture verifies connected T3 expiry produces RR diagnostic action and plan. |
| T2 placeholder replay          | scaffold    | Fixture verifies T2 placeholder expiry is reported as planned. Delayed ACK is not implemented. |
| N2 replay                      | implemented | Replay fixtures assert retry increments and retry count state. |
| Timers                         | partial     | Logical timers exist for tests. No daemon scheduler is started. |
| Retries                        | partial     | N2 retry count is used by isolated timeout transitions and scheduler diagnostics. |
| Modulo 8                       | implemented | Basic sequence increment, receive accounting, and ack handling are tested. |
| Modulo 128                     | planned     | Parameter placeholder exists. Extended control decode is deferred. |
| FRMR handling                  | partial     | Control subtype and protocol-error action placeholder exist. |
| TEST/XID handling              | scaffold    | Control subtype names exist. No procedures are implemented. |
| Live connected-mode sessions   | planned     | CONNECT, RF shell binding, BBS binding, and dispatch integration are not implemented. |
| Control-plane connection diagnostics | implemented | Read-only control socket commands are exposed. No mutation commands exist. |
| Live daemon RX integration     | implemented | Feed is wired behind disabled-by-default config gates. |
| Live CONNECT                   | planned     | No shell, BBS, or RF command exposes connect behaviour. |
| Live response queueing         | planned     | Generated frame plans are not queued or dispatched. |
| TX response from bench         | planned     | Receive-only bench configs do not enable TX queue writes or dispatch. |
| Payload delivery               | planned     | I-frame payload bytes are not delivered to shell or BBS code. |
| Live scheduler policy          | implemented | Disabled-by-default policy validates safe diagnostic-only combinations. |
| Live scheduler diagnostics     | implemented | Read-only status, timer list, and counter formatters are exposed. |
| Live scheduler timer list      | implemented | Control output lists bounded logical timer rows without mutation. |
| Live scheduler counters        | implemented | Counters include cycles, expiries, blocked actions, plans, and TX writes. |
| Live poll helper               | scaffold    | Explicit poll helper can process bounded expiries when enabled by config. |
| Timer scheduler                | scaffold    | Runtime owns a disabled scheduler scaffold and live diagnostic wrapper. |
| Live daemon timer integration  | partial     | Daemon owns the wrapper and can call explicit polling, but no TX bridge exists. |
| Live scheduler smoke mode      | implemented | Disabled-by-default daemon smoke mode polls scheduler diagnostics and keeps TX writes zero. |
| Daemon scheduler poll helper   | implemented | Daemon calls a wrapper that supports normal live scheduler polling and smoke diagnostics. |
| Scheduler smoke counters       | implemented | Control output reports smoke cycles, test connections, expiries, prepared frames, bridge blocks, TX writes, and dispatch attempts. |
| No-transmit scheduler proof    | implemented | Smoke tests and scripts assert TX writes and dispatch attempts stay zero. |
| Retransmission dispatch        | planned     | Timeout send actions are not queued or dispatched. |
| Action-plan to TX queue bridge | planned     | Frame plans are retained for diagnostics only. |
| Prepared response frame queue | implemented | Bounded diagnostics queue stores generated AX.25 response frame plans and optional raw AX.25 bytes. |
| Raw AX.25 response bytes      | implemented | Existing frame builder output is retained for inspection only, without HDLC flags, FCS, KISS, or FX.25. |
| Prepared frame diagnostics control plane | implemented | Read-only `AX25 PREPARED` commands list prepared diagnostic records and counters. |
| Prepared frame replay assertions | implemented | Bench and timer replay can assert expected prepared diagnostic frames without TX queue writes. |
| Bench prepared assertion fixtures | implemented | `prepared-frames.expected` checks UI captures produce no prepared frames and SABM captures produce UA diagnostics. |
| Timer prepared assertion fixtures | implemented | Timer replay scripts assert prepared retry/poll/disconnect frame diagnostics. |
| Manual capture prepared assertion support | partial | Manual replay reports prepared frame counts; optional reviewed expectations remain file-based workflow. |
| Prepared replay TX writes zero assertion | implemented | Prepared replay checks fail if any TX write counter becomes non-zero. |
| Prepared-to-TX bridge blocked | implemented | Bridge helper returns blocked and config rejects `prepared-bridge-to-tx true`. |
| Prepared-to-TX bridge gate | implemented | Disabled-by-default gate evaluates prepared AX.25 frames without real TX queue writes. |
| Bridge decision diagnostics | implemented | Read-only control output reports bridge policy, per-frame decision, and counters. |
| Test-only TX frame conversion | implemented | Unit tests can build an in-memory dry-run TX frame from a prepared frame after all gates pass. |
| Prepared bridge runtime disabled | implemented | Runtime bridge calls remain blocked and do not enqueue frames. |
| Response safety checklist       | implemented | Safety docs define protocol, bench, operator, failure, and deferred blocker gates. |
| No-transmit regression checks   | implemented | Shell checks verify default examples and replay outputs keep TX writes at zero. |
| Prepared-to-TX gate report      | implemented | Non-transmitting report script verifies the prepared bridge stays blocked. |
| Future response TX bench gate   | implemented | Bench docs define Stage 0/1 current work and Stage 2/3 blocked future work. |
| Real TX queue bridge          | planned     | Prepared frames are not copied to the real TX queue. |
| Real response TX               | blocked     | Requires safety checklist completion and a separate implementation milestone. |
| Dispatch planned              | planned     | Prepared bridge diagnostics do not run TX dispatch. |
| Timers real scheduling         | planned     | Timeout events are unit-test/offline inputs only. No live scheduler is started. |
| FCS handling                   | deferred    | KISS-facing code operates at AX.25 body boundary. |
| KISS payload boundary          | implemented | Existing KISS encode/decode treats AX.25 bytes as payload. |
