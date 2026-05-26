# TNC Interface Matrix

Status values: `planned`, `partial`, `implemented`, `tested`.

| Interface | Status | Notes |
| --------- | ------ | ----- |
| KISS stream parsing | tested | Incremental parser with escape and size checks |
| AX.25 monitor decode | tested | Diagnostic UI frame formatting |
| KISS stdin/stdout | implemented | Live monitor can read stdin KISS streams |
| KISS TCP client | partial | Blocking monitor client, no reconnect or auth |
| KISS TCP server | partial | Blocking single-client monitor server |
| KISS serial | implemented | Monitor mode plus validation tests, no hardware CI |
| KISS PTY | implemented | PTY creation and transfer tests where supported |
| Unix socket KISS | implemented | Local stream socket client/server tests |
| KiloTNC | partial | Serial KISS can consume it, dedicated tests pending |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA external modem | planned | External modem adapter target |
| ARDOP external modem | planned | External modem adapter target |
| Pactor external modem | planned | External modem adapter target |
