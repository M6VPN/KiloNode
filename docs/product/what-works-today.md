# What Works Today

The current KiloNode preview supports these areas:

| Area | Status | Notes |
| ---- | ------ | ----- |
| Build and tests | Working | Local CMake build and deterministic tests. |
| Local daemon config check | Working | `kilonoded --check-config` validates examples. |
| Local control socket | Working | Read-only and guarded mutating commands as documented. |
| Local node shell | Working | Localhost shell in hobbyist examples. |
| Local BBS store | Working | Local storage and maintenance tools. |
| BBS maintenance tools | Working | Local control and store utilities. |
| KISS receive monitor | Working | Receive-side KISS/AX.25 diagnostics. |
| Dire Wolf TCP KISS receive-only | Usable | First practical hobbyist receive path. |
| KiloTNC receive path | Documented | Manual hardware path, receive-only safety applies. |
| AX.25 loopback simulator | Working | In-memory connected-mode diagnostics. |
| AX.25 I-frame loopback | Working | Simulator-only payload delivery diagnostics. |
| AX.25 segmentation loopback | Working | Simulator-only paclen and reassembly diagnostics. |
| CONNECT dry-run planning | Working | No live CONNECT command exposed. |
| External modem status profiles | Working | Read-only profile and configured modem status. |
| Mercury OFDM planned profile | Working | Status-only and discovery docs, no implementation. |
| No-transmit safety checks | Working | Repository configs are checked for blocked TX paths. |

Live RF connected-mode sessions are not implemented.
