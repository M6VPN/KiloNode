# LinBPQ Compatibility Matrix

Status values: `planned`, `partial`, `implemented`, `tested`.

| Feature | Status | Notes |
| ------- | ------ | ----- |
| AX.25 UI frame encode/decode | tested | Address lists, PID, digipeaters, and binary payloads |
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
| Node shell users | partial | Local shell sessions only |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA external modem | planned | External modem adapter target |
| ARDOP external modem | planned | External modem adapter target |
| Pactor external modem | planned | External modem adapter target |
