# TNC Interface Matrix

Status values: `planned`, `partial`, `implemented`, `tested`.

| Interface | Status | Notes |
| --------- | ------ | ----- |
| KISS stream parsing | tested | Incremental parser with escape and size checks |
| AX.25 monitor decode | tested | Diagnostic UI frame formatting |
| Live transport stats | implemented | Query with `kilonodectl stats` and `ports` |
| Heard tracking | tested | Decoded AX.25 frames from any configured KISS transport |
| RX event queue | tested | Decoded frames from configured KISS transports feed recent receive events |
| RX observed sessions | tested | Receive-side source/destination observations from decoded frames |
| KISS stdin/stdout | implemented | Live monitor can read stdin KISS streams |
| KISS TCP client | partial | Configured daemon and monitor support, no reconnect or auth |
| KISS TCP server | partial | Configured daemon and monitor support, single-client only |
| KISS serial | partial | Configured daemon and monitor support, no hardware CI |
| KISS PTY | implemented | Configured daemon and monitor support with PTY tests |
| Unix socket KISS | implemented | Configured daemon and monitor support with local tests |
| KiloTNC | partial | Serial KISS can consume it, dedicated tests pending |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA external modem | planned | External modem adapter target |
| ARDOP external modem | planned | External modem adapter target |
| Pactor external modem | planned | External modem adapter target |
