# M1 Feature Inventory

| Feature | Status | Primary tests | Primary docs | Limitations |
|---------|--------|---------------|--------------|-------------|
| Core C17 build | implemented | `ctest`, CI workflows | `README.md` | No external runtime deps. |
| Control socket | implemented | `test_control_protocol`, `test_control_socket` | `docs/man/kilonodectl.1` | Local control only. |
| KISS transports | implemented | `test_kiss`, `test_kiss_stream`, transport tests | `docs/protocols/kiss-transport-notes.md` | TX dispatch remains gated. |
| KISS monitor | implemented | `test_monitor` | `docs/man/kilonode-monitor.1` | Monitor is inspection tooling. |
| AX.25 UI parsing and monitor | implemented | `test_ax25`, `test_ax25_rx_feed` | `docs/protocols/ax25-notes.md` | Connected sessions not live. |
| RX event queue and heard list | implemented | `test_rx_event`, `test_rx_queue`, `test_heard` | `docs/bench/README.md` | Receive-side only. |
| Local node shell | implemented | `test_node_shell`, `test_node_command_dispatch` | `docs/man/kilonoded.8` | No CONNECT command. |
| Local BBS store and shell | implemented | `test_bbs_*`, `test_message_store` | `docs/man/kilonode-msg.1`, `docs/man/kilonode-store.1` | No RF BBS access. |
| BBS users and read-state | implemented | `test_bbs_user`, `test_bbs_read_state` | `docs/man/kilonode-user.1` | Local identity only. |
| BBS maintenance/control | implemented | `test_bbs_store_maintenance`, `test_bbs_control` | `docs/man/kilonodectl.1` | Read-only control queries. |
| RF command ingress | implemented | `test_rf_command`, `test_daemon_rf_command` | `docs/compat/rf-command-transcript-format.md` | Replies remain policy gated. |
| RF abuse controls | implemented | `test_rf_abuse`, `test_rf_ignore` | `docs/man/kilonodectl.1` | Local policy only. |
| TX dry-run and gates | implemented | `test_tx_*`, `test_daemon_tx` | `docs/safety/ax25-no-transmit-regression.md` | Real TX is blocked by default. |
| AX.25 connected-mode diagnostics | partial | `test_ax25_state`, `test_ax25_connection_table` | `docs/protocols/ax25-connected-mode-plan.md` | No live connected sessions. |
| AX.25 timer and scheduler scaffolds | implemented | `test_ax25_timer*`, `test_ax25_scheduler*` | `docs/design/081-088*.md` | No real retransmission dispatch. |
| Live scheduler smoke diagnostics | implemented | `test_ax25_scheduler_smoke`, `test_daemon_ax25_scheduler` | `docs/design/106-108*.md` | Disabled by default. |
| Prepared-frame diagnostics | implemented | `test_ax25_prepared_*` | `docs/design/091-097*.md` | Diagnostics queue only. |
| Prepared-to-TX bridge gate | implemented | `test_ax25_prepared_tx_*` | `docs/design/098-101*.md` | Runtime bridge remains blocked. |
| Bench capture tooling | implemented | `test_compat_bench_pack`, `test_bench_*` | `docs/bench/` | Synthetic and manual receive-only. |
| Manual capture workspace | implemented | `test_manual_capture_*` | `docs/bench/manual-capture-*.md` | Manual captures are not committed automatically. |
| Compatibility lab tooling | implemented | `test_compat_*` | `docs/compat/` | Actual LinBPQ interop remains planned. |
| Packaging examples | implemented | `scripts/check-packaging.sh` | `packaging/examples/README.md` | Examples do not install services automatically. |
| FX.25 scaffolding | scaffold | `test_fx25`, `test_fx25_params` | `docs/protocols/fx25-layering-plan.md` | No FEC encode/decode. |
