# TNC Interface Matrix

Status values: `planned`, `partial`, `implemented`, `tested`.

| Interface | Status | Notes |
| --------- | ------ | ----- |
| KISS stream parsing | tested | Incremental parser with escape and size checks |
| AX.25 monitor decode | tested | Diagnostic UI frame formatting |
| KISS stdin/stdout | tested | Live monitor can read stdin KISS streams |
| KISS TCP client | partial | Blocking monitor client, no reconnect or auth |
| KISS TCP server | partial | Blocking single-client monitor server |
| KISS serial | planned | Transport adapter not started |
| KISS PTY | planned | Transport adapter not started |
| Unix socket KISS | planned | Transport adapter not started |
| KiloTNC | planned | First-class support goal through future adapters |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA external modem | planned | External modem adapter target |
| ARDOP external modem | planned | External modem adapter target |
| Pactor external modem | planned | External modem adapter target |
