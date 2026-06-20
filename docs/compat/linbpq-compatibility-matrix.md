# LinBPQ Compatibility Matrix

Status values: `planned`, `partial`, `implemented`, `tested`.

| Feature | Status | Notes |
| ------- | ------ | ----- |
| AX.25 UI frame encode/decode | tested | Address lists, PID, digipeaters, and binary payloads |
| AX.25 control classification scaffold | tested | UI, I, S, and U class helpers compile and test |
| AX.25 connected-mode scaffold | tested | Disabled structs, params, event names, and formatting only |
| AX.25 connected-mode state-machine core | tested | Isolated unit-tested state core only, not wired to runtime |
| AX.25 modulo 8 sequence accounting | tested | Basic receive and acknowledgement helpers |
| Internal AX.25 segmented loopback | tested | KiloNode loopback splits paclen-sized I frames, records reassembly diagnostics, and returns RR frames |
| BPQ/LinBPQ paclen compatibility | planned | Future black-box observation only, no BPQ/LinBPQ config or segmentation logic copied |
| Live RF payload segmentation | planned | Not implemented |
| AX.25 action intents | tested | Bounded intents only, no frame bytes or TX queue writes |
| AX.25 response frame builder | tested | Raw AX.25 SABM/SABME/UA/DM/DISC/RR/RNR/REJ builder only |
| AX.25 action-to-frame mapper | tested | Action intents map to frame plans only, no queue or dispatch |
| AX.25 connection table scaffold | tested | Bounded key and table layer feeds isolated state core and retains frame plans |
| AX.25 connection diagnostics | tested | Read-only formatters for keys, records, actions, frame plans, and table summary |
| AX.25 timer scaffold | tested | Logical T1/T2/T3 timers only, no OS timers or live polling |
| AX.25 retry scaffold | tested | Bounded N2 helper for offline scheduler diagnostics |
| AX.25 scheduler scaffold | tested | Applies timer/retry action intents and produces timeout events without TX |
| AX.25 timer replay harness | tested | Offline timer scripts validate T1/T2/T3 diagnostics without live sessions |
| AX.25 retry diagnostics | tested | Replay fixtures assert N2 retry and exhaustion behaviour |
| AX.25 live scheduler boundary | tested | Disabled-by-default daemon wrapper and read-only diagnostics only |
| AX.25 scheduler control diagnostics | tested | Scheduler status, timer rows, and counters are read-only |
| Live AX.25 connection table in daemon | planned | Not implemented |
| AX.25 live response queueing | planned | Not implemented |
| Live AX.25 retransmission | planned | Timeout send intents are not queued or dispatched |
| Live BPQ/LinBPQ connected-mode timer interop | planned | Future black-box tests only |
| AX.25 connected-mode state machine | partial | First core exists, live sessions are planned |
| Live AX.25 connected sessions | planned | Not implemented |
| RF CONNECT command | planned | Not implemented |
| FX.25 reference indexing | implemented | Local FX.25 reference is indexed for future work |
| FX.25 layer scaffold | tested | Disabled params and decode placeholder only |
| FX.25 FEC encode/decode | planned | Not implemented |
| FX.25 wrapping | planned | Not implemented |
| FX.25 KISS integration | planned | Not implemented |
| BPQ/LinBPQ connected-mode interop | planned | Future black-box tests only |
| BPQ/LinBPQ FX.25 interop if applicable | planned | Future black-box tests only |
| Native KiloNode config | tested | Clean-room parser for node and KISS port blocks |
| BPQ-style config import | planned | Future compatibility tool, not native syntax |
| Foreground daemon | implemented | `kilonoded --foreground` with signal shutdown |
| Multi-port daemon monitor | partial | Poll loop over enabled KISS ports, no reconnect |
| Native control socket | implemented | Local Unix socket, read-only commands |
| kilonodectl status | implemented | Local status, ports, stats, and ping commands |
| Runtime port stats | implemented | Basic global and per-port counters |
| Heard list | tested | Per-port AX.25 source observation state |
| Heard list via control socket | tested | `kilonodectl heard` and port filtering |
| BPQ-compatible MHEARD command | planned | Compatibility command not implemented |
| Native local node shell | tested | Local TCP diagnostic shell |
| HELP command | tested | KiloNode-native shell command |
| INFO command | tested | Shows node metadata |
| PORTS command | tested | Shows configured port state |
| HEARD command | tested | Uses daemon heard-list state |
| USERS command | partial | Local shell sessions only |
| BPQ-style command compatibility | planned | Not implemented |
| RF CONNECT command | planned | Packet transmit not implemented |
| NET/ROM NODES/ROUTES | planned | Routing state not implemented |
| BBS handoff | planned | Mailbox not implemented |
| KiloNode-native message store | tested | Filesystem-backed local store |
| Private message storage | tested | Create, list, read, and soft delete |
| Bulletin storage | tested | Create, list, read, and soft delete |
| Message deletion | partial | Soft delete only |
| KiloNode-native message indexes | tested | Index rebuild, filters, and corruption recovery |
| KiloNode-native store check | tested | Integrity findings for native store files |
| KiloNode-native store repair | tested | Safe directory, next-id, read-state, and index repair |
| KiloNode-native store reindex | tested | Rebuilds native index files |
| KiloNode-native store export | tested | Directory snapshot with manifest |
| Soft-delete purge | tested | Removes messages already marked deleted |
| KiloNode-native BBS control status | tested | Read-only `BBS STATUS` over local control socket |
| KiloNode-native BBS control stats | tested | Read-only store counts through `kilonodectl bbs stats` |
| KiloNode-native BBS control areas | tested | Indexed bulletin area summaries |
| KiloNode-native BBS control users | tested | Local user summaries, no passwords |
| KiloNode-native BBS control message summaries | tested | Filtered message summaries and bounded previews |
| KiloNode-native access policy | tested | Native policy block for local shell, BBS, and control limits |
| Shell idle timeout | tested | Inactive local shell sessions close cleanly |
| Shell input rate limit | tested | Per-session command rate window |
| BBS body size enforcement | tested | Oversized multiline SEND bodies are rejected before storage |
| Control command size limit | tested | Overlong local control commands are rejected |
| CI normal build | tested | GCC and Clang GitHub Actions workflow |
| CI sanitizer build | tested | ASan and UBSan workflow |
| Hardened release build | tested | Optional CMake hardening profile |
| Platform portability policy | partial | Linux tested, BSD targets documented |
| RF receive event model | tested | Receive-only decoded AX.25 event queue |
| RX event control queries | tested | Read-only `kilonodectl rx events` queries |
| Observed RF session table | tested | Source/destination observations, no state machine |
| AX.25 UI outbound builder | tested | Builds bounded UI frames through existing AX.25 encoder |
| KISS outbound encoding | tested | Stores complete escaped KISS frames for prepared TX frames |
| Transmit queue skeleton | tested | In-memory queue with read-only control queries |
| Dry-run UI enqueue path | tested | Local control-gated dry-run queue insertion only |
| Control TX dry-run diagnostics | tested | `kilonodectl tx dryrun-ui`, `tx queue`, and `tx frame` |
| TX dispatch test harness | tested | Test-only memory dispatch, no RF writes |
| Memory/mock transport | tested | In-process bounded byte sink for dispatch tests |
| Real KISS TX dispatch | partial | Control-triggered only and blocked unless all safety gates pass |
| TX safety gates | tested | Global transmit gates plus per-port `tx-enabled` |
| Local-admin TX dispatch command | tested | `kilonodectl tx dispatch-run` only, no shell or BBS TX |
| RF UI command ingress | tested | UI-frame-only KiloNode-native command parser |
| KiloNode-native RF HELP/INFO/PORTS/HEARD/STATS/PING | tested | Minimal receive-side command set |
| Gated RF UI replies | partial | Replies queue only when RF and TX gates allow it, no auto dispatch |
| RF source rate limiting | tested | Per-source runtime command windows |
| RF reply suppression | tested | Reply window suppresses TX queue insertion |
| RF ignore list | tested | Native manual ignore file and diagnostics |
| RF auto-ignore | tested | Runtime temporary ignore after repeated rejects |
| KiloNode synthetic RF command fixtures | tested | Synthetic KiloNode-native baseline transcripts |
| Compatibility transcript parser | tested | Native line-based transcript parser |
| Compatibility replay harness | tested | Synthetic RF UI replay without real transports |
| Black-box observation parser | tested | Native observation file parser |
| Process observation harness | tested | Explicit binary path only, no shell |
| TCP observation harness | tested | Explicit endpoint, bounded line capture |
| Transcript generation from observations | tested | Generates marked candidate data |
| Packet capture format | tested | Native packet-boundary text captures |
| KISS capture parser | tested | Offline KISS frame decode and expectations |
| AXIP/AXUDP capture parser | tested | Offline raw AX.25 payload observations |
| Capture expectation reports | tested | Deterministic pass/fail mismatch reports |
| Capture to transcript conversion | tested | RF UI payload candidate generation |
| Node observation pack manifest | tested | Native manifest parser with clean-room checks |
| Synthetic node observation fixtures | tested | Placeholder observations for tooling coverage |
| Node command coverage report | tested | Deterministic coverage summary |
| Node command requirements layer | tested | Planning file parser and coverage cross-check |
| Command profile planner | tested | External behaviour categories only, no parser logic |
| Compatibility risk register | tested | Default clean-room and prerequisite risk report |
| Generated node plan from observations | tested | Writes conservative requirements and profiles |
| KiloNode-native command dispatcher | tested | Shared runtime dispatcher for native commands |
| Local/RF command profile sharing | partial | Local shell and RF UI share native profile definitions |
| BPQ/LinBPQ command aliases | planned | Not implemented |
| BPQ/LinBPQ command output compatibility | planned | Not implemented |
| Connected-mode command context | planned | Not implemented |
| NET/ROM node command context | planned | Not implemented |
| RF BBS command context | planned | Not implemented |
| Manual LinBPQ node observations | planned | Future explicit black-box captures |
| Live KISS capture | planned | Not implemented |
| Live AXIP capture | planned | Not implemented |
| BPQ/LinBPQ packet observations | planned | Future manual black-box captures |
| Automated LinBPQ compatibility observations | planned | Manual explicit action only |
| Black-box LinBPQ execution harness | partial | Optional explicit CLI, not used by tests |
| BPQ/LinBPQ RF command observations | planned | Future black-box transcripts |
| BPQ/LinBPQ command compatibility implementation | planned | Not implemented |
| RF BBS compatibility observations | planned | Future black-box transcripts |
| BBS forwarding compatibility observations | planned | Future protocol-boundary observations |
| AX.25 connected-mode receive state | planned | Not implemented |
| Real KISS transport TX dispatch | partial | Implemented behind disabled-by-default gates |
| Actual RF transmit dispatch | partial | Control-triggered lab path only, no scheduler |
| AX.25 transmit path | partial | Builder and queue only, no dispatch |
| AX.25 read-only diagnostics control plane | tested | Runtime status, params, connection list, detail, and counters only |
| AX.25 runtime scaffold | tested | Daemon-owned scaffold exists with live processing disabled |
| AX.25 live RX diagnostics feed | tested | Decoded AX.25 connected-mode frames can update diagnostics when explicitly enabled |
| AX.25 live connection diagnostics from RX | partial | SABM/SABME creation and existing-record updates are diagnostic only |
| Receive-only AX.25 bench validation docs | implemented | Bench docs, examples, and helper scripts cover RX-only validation paths |
| Receive-only bench capture fixture pack | tested | Synthetic KISS and AX.25 bench captures replay without hardware |
| Offline AX.25 diagnostic replay from bench captures | tested | Bench captures feed the AX.25 diagnostics runtime and assert zero TX writes |
| Manual black-box KISS captures import | tested | Manual workspace imports receive-only captures without using them as implementation sources |
| Dire Wolf/KISS receive bench plan | implemented | USB sound card to Dire Wolf TCP KISS to KiloNode workflow documented |
| KiloTNC receive bench plan | implemented | Serial and TCP KISS receive-only workflow documented |
| Live LinBPQ/KISS receive observation | planned | Future manual black-box observation only |
| BPQ/LinBPQ connected-mode capture comparison | planned | Future manual captures only, no compatibility implementation in this pass |
| Manual LinBPQ/KISS connected-mode capture replay | planned | Future receive-only black-box captures only |
| Live LinBPQ interop | planned | Not implemented |
| Live AX.25 connected sessions | planned | Not implemented |
| RF CONNECT command | planned | Not implemented |
| BPQ/LinBPQ connected-mode interop | planned | Not implemented |
| FX.25 FEC | planned | Not implemented |
| RF node shell transmit | planned | Not implemented |
| RF BBS replies | planned | Not implemented |
| AX.25 connected-mode transmit | planned | Not implemented |
| NET/ROM transmit/routing | planned | Not implemented |
| RF node shell access | planned | Not implemented |
| BPQ/LinBPQ node command compatibility | planned | Not implemented |
| Bulletin area discovery | tested | Indexed area list with counts and newest ID |
| KiloNode-native local BBS shell | tested | Local node shell BBS mode |
| Local LIST command | tested | Lists non-deleted message summaries |
| LIST PRIVATE | tested | Indexed private message filter |
| LIST BULLETINS | tested | Indexed bulletin filter |
| LIST AREA | tested | Indexed bulletin area filter |
| LIST TO/FROM filters | tested | Indexed callsign filters |
| Local READ command | tested | Reads metadata and body through BBS mode |
| Read flag | tested | Per-user read markers for local BBS identities |
| Local SEND PRIVATE command | tested | Creates private messages through BBS mode |
| Local SEND BULLETIN command | tested | Creates bulletins through BBS mode |
| Local KILL command | tested | Soft-deletes messages through BBS mode |
| KiloNode-native BBS users | tested | Local callsign identities, no passwords |
| Callsign identity in local BBS | tested | `BBS <callsign>` enters identity-aware mode |
| Per-user read state | tested | Read files under the message store |
| Unread list | tested | `UNREAD` lists messages unread by current identity |
| BPQ/LinBPQ user compatibility | planned | Not implemented |
| BPQ/LinBPQ topic/read-state compatibility | planned | Not implemented |
| BPQ/LinBPQ topic compatibility | planned | Future black-box research |
| RF BBS access | planned | Not implemented |
| RF BBS login | planned | Not implemented |
| BPQ/LinBPQ node interop | planned | Future black-box tests only |
| BBS forwarding identity/state | planned | Not implemented |
| BPQ/LinBPQ BBS command compatibility | planned | Not implemented |
| BPQ mailbox command compatibility | planned | Not implemented |
| BPQ/LinBPQ message import | planned | Not implemented |
| BPQ/LinBPQ mailbox import/export | planned | Not implemented |
| BPQ/LinBPQ mailbox import | planned | Not implemented |
| BPQ/LinBPQ mailbox export | planned | Not implemented |
| BPQ/LinBPQ remote sysop status commands | planned | Not implemented |
| BPQ/LinBPQ sysop authentication compatibility | planned | Not implemented |
| BPQ/LinBPQ user/password compatibility | planned | Not implemented |
| BPQ/LinBPQ access-control compatibility | planned | Not implemented |
| RF BBS access control | planned | Not implemented |
| BBS forwarding | planned | Not implemented |
| BBS forwarding queue | planned | Not implemented |
| BBS forwarding queues | planned | Not implemented |
| FBB forwarding compatibility | planned | Not implemented |
| BBS forwarding status | planned | Not implemented |
| FBB forwarding status | planned | Not implemented |
| BPQ console compatibility | planned | Not implemented |
| BPQ-style remote sysop commands | planned | Not implemented |
| Telnet user access | partial | Local TCP shell only, no auth or public mode |
| BBS mailbox | partial | Local BBS shell and store exist, RF access planned |
| NET/ROM node behaviour | planned | Black-box tests required |
| NET/ROM route table | planned | Routing state not implemented |
| AX.25 prepared response diagnostics queue | implemented | Generated AX.25 response plans and bytes are retained for inspection only if tests pass. |
| AX.25 response bytes for diagnostics | implemented | Raw AX.25 body bytes can be built without TX queue writes if tests pass. |
| AX.25 prepared-frame replay assertions | implemented | Offline bench and timer replay can compare expected prepared diagnostics if tests pass. |
| AX.25 prepared-to-TX bridge gate | implemented | Disabled diagnostic gate evaluates prepared frames without real TX queue writes if tests pass. |
| AX.25 response safety checklist | implemented | Docs and scripts define blockers before any real connected-mode response TX. |
| AX.25 live scheduler smoke mode | implemented | Daemon smoke diagnostics can poll AX.25 timers without TX queue writes if tests pass. |
| M1 compatibility audit | implemented | Milestone docs and scripts validate clean-room fixtures, observation packs, and planning files without running LinBPQ. |
| v0.1-alpha receive/diagnostics readiness | implemented | Recommended scope is native receive, diagnostics, local BBS/control, compatibility lab tooling, and safety gates only. |
| KiloNode internal AX.25 loopback simulator | implemented | M2.1 runs two native endpoints in memory without LinBPQ, real KISS, RF, TX queue writes, or dispatch. |
| AX.25 SABM/UA loopback | tested | Native loopback fixtures prove setup state transitions and prepared diagnostics only. |
| AX.25 DISC/UA loopback | tested | Native loopback fixtures prove disconnect state transitions and no-transmit counters. |
| AX.25 I/RR loopback | partial | Simulator-local I frame exchange is tested. Live connected-mode I-frame TX remains planned. |
| Internal AX.25 I/RR payload loopback | implemented | M2.2 tests bounded text and binary payload diagnostics plus RR acknowledgement without live RF. |
| Internal AX.25 windowed loopback | implemented | M2.4 tests native outstanding-frame diagnostics and windowed segmented sends without LinBPQ or RF. |
| Internal AX.25 retransmission-buffer loopback | implemented | M2.5 tests native REJ-driven replay diagnostics without LinBPQ, RF, real TX queue writes, or dispatch. |
| Internal AX.25 CONNECT dry-run | implemented | M2.6 validates KiloNode-native CONNECT intent offline without LinBPQ, live CONNECT, RF, TX queue writes, or dispatch. |
| BPQ/LinBPQ connected-mode payload interop | planned | Future black-box captures only. No LinBPQ code or command compatibility is used. |
| Live RF connected-mode loopback | planned | Future work requires real TX safety gates and black-box interop planning. |
| Manual LinBPQ prepared response comparison | planned | Future receive-only black-box captures may be compared manually. |
| Real connected-mode response TX | planned | Prepared diagnostics are not transmitted. |
| BPQ/LinBPQ connected-mode response TX | blocked | Requires future black-box interop work and safety gate completion. |
| BPQ/LinBPQ connected-mode interop | planned | Future black-box tests only. |
| Node shell users | partial | Local shell sessions only |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA FM external modem | planned | Future external modem adapter target, not BPQ/LinBPQ command compatibility |
| VARA HF external modem | planned | Future external modem adapter target, not BPQ/LinBPQ command compatibility |
| Mercury OFDM external modem | planned | Future Rhizomatica Mercury adapter target kept outside KiloNode source |
| ARDOP external modem | planned | Future external TCP host adapter target |
| Pactor external modem | planned | External modem adapter target |
