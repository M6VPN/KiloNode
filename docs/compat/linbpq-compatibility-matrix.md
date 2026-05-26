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
| BPQ console compatibility | planned | Not implemented |
| BPQ-style remote sysop commands | planned | Not implemented |
| Telnet user access | planned | Local node shell target |
| BBS mailbox | planned | Message store not started |
| NET/ROM node behaviour | planned | Black-box tests required |
| FBB forwarding | planned | Later BBS interop target |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA external modem | planned | External modem adapter target |
| ARDOP external modem | planned | External modem adapter target |
| Pactor external modem | planned | External modem adapter target |
