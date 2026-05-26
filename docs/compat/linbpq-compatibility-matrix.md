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
| BPQ console compatibility | planned | Not implemented |
| BPQ-style remote sysop commands | planned | Not implemented |
| Telnet user access | partial | Local TCP shell only, no auth or public mode |
| BBS mailbox | planned | Message store not started |
| NET/ROM node behaviour | planned | Black-box tests required |
| NET/ROM route table | planned | Routing state not implemented |
| Node shell users | partial | Local shell sessions only |
| FBB forwarding | planned | Later BBS interop target |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA external modem | planned | External modem adapter target |
| ARDOP external modem | planned | External modem adapter target |
| Pactor external modem | planned | External modem adapter target |
