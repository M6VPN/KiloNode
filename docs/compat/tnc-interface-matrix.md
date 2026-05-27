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
| TX queue skeleton | tested | Outbound frames can be built and queued, dispatch is disabled |
| TX dry-run enqueue | tested | Builds AX.25/KISS frames and queues them without writing to TNC transports |
| TX dispatch test harness | tested | Memory/mock-only dispatch path, no real TNC writes |
| Real KISS TX dispatch | partial | Control-triggered and blocked unless all global and per-port TX gates pass |
| KISS stdin/stdout | implemented | RX supported, real TX blocked by default |
| KISS TCP client | partial | RX supported, TX gated by `tx-enabled` and transmit policy |
| KISS TCP server | partial | RX supported, TX gated after an accepted client exists |
| KISS serial | partial | RX supported, TX gated, no hardware CI |
| KISS PTY | implemented | RX supported, TX gated with local write-path tests |
| Unix socket KISS | implemented | RX supported, TX gated after socket connect or accept |
| KiloTNC | partial | Serial KISS can consume it, dedicated tests pending |
| AXIP/AXUDP | planned | Later IP transport target |
| AGWPE TCP | planned | Later soundmodem compatibility target |
| BPQEther | planned | Later Ethernet compatibility target |
| VARA external modem | planned | External modem adapter target |
| ARDOP external modem | planned | External modem adapter target |
| Pactor external modem | planned | External modem adapter target |
