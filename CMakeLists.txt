# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(KiloNode VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(cmake/KiloNodeHardening.cmake)
include(cmake/KiloNodeInstall.cmake)
include(cmake/KiloNodeSanitizers.cmake)
include(cmake/KiloNodeWarnings.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

function(kilonode_target_defaults target_name)
	target_compile_features(${target_name} PRIVATE c_std_17)
	kilonode_apply_warnings(${target_name})
	kilonode_apply_sanitizers(${target_name})
	kilonode_apply_hardening(${target_name})
endfunction()

add_library(packetcore STATIC
	src/compat/bench_ax25_diag_replay.c
	src/compat/bench_replay.c
	src/compat/bench_replay_report.c
	src/compat/compat_bench_pack.c
	src/compat/compat_capture.c
	src/compat/compat_capture_report.c
	src/compat/compat_axip_capture.c
	src/compat/compat_command_profile.c
	src/compat/compat_coverage.c
	src/compat/compat_kiss_capture.c
	src/compat/compat_observe.c
	src/compat/compat_observation_pack.c
	src/compat/compat_packet_capture.c
	src/compat/compat_process.c
	src/compat/compat_requirements.c
	src/compat/compat_report.c
	src/compat/compat_replay.c
	src/compat/compat_risk.c
	src/compat/compat_session.c
	src/compat/compat_transcript.c
	src/compat/manual_capture_index.c
	src/compat/manual_capture_replay.c
	src/compat/manual_capture_report.c
	src/compat/manual_capture_workspace.c
	src/control/control_protocol.c
	src/control/control_socket.c
	src/bbs/bbs_area.c
	src/bbs/bbs_control.c
	src/bbs/bbs_read_state.c
	src/bbs/bbs_shell.c
	src/bbs/bbs_store_lock.c
	src/bbs/bbs_store_maintenance.c
	src/bbs/bbs_user.c
	src/bbs/message.c
	src/bbs/message_index.c
	src/bbs/message_store.c
	src/core/access_policy.c
	src/core/buffer.c
	src/core/callsign.c
	src/core/config.c
	src/core/heard.c
	src/core/session_limits.c
	src/core/stats.c
	src/daemon/daemon.c
	src/monitor/monitor.c
	src/node/node_command.c
	src/node/node_command_dispatch.c
	src/node/node_command_profile.c
	src/node/node_shell.c
	src/proto/ax25.c
	src/proto/ax25_action.c
	src/proto/ax25_action_mapper.c
	src/proto/ax25_control_plane.c
	src/proto/ax25_control.c
	src/proto/ax25_connection.c
	src/proto/ax25_connection_diag.c
	src/proto/ax25_connection_event.c
	src/proto/ax25_connection_key.c
	src/proto/ax25_connection_table.c
	src/proto/ax25_frame_builder.c
	src/proto/ax25_frame_plan.c
	src/proto/ax25_live_diag.c
	src/proto/ax25_params.c
	src/proto/ax25_retry.c
	src/proto/ax25_rx_feed.c
	src/proto/ax25_runtime.c
	src/proto/ax25_scheduler.c
	src/proto/ax25_sequence.c
	src/proto/ax25_state.c
	src/proto/ax25_timer.c
	src/proto/ax25_timer_queue.c
	src/proto/fx25.c
	src/proto/fx25_params.c
	src/proto/kiss.c
	src/proto/kiss_stream.c
	src/rf/rf_command.c
	src/rf/rf_command_queue.c
	src/rf/rf_abuse.c
	src/rf/rf_ignore.c
	src/rf/rf_reply.c
	src/rf/rx_event.c
	src/rf/rx_queue.c
	src/rf/rx_session.c
	src/rf/tx_dispatch.c
	src/rf/tx_dry_run.c
	src/rf/tx_frame.c
	src/rf/tx_policy.c
	src/rf/tx_queue.c
	src/rf/tx_transport_gate.c
	src/transport/transport.c
	src/transport/transport_memory.c
	src/transport/transport_pty.c
	src/transport/transport_serial.c
	src/transport/transport_stdio.c
	src/transport/transport_tcp.c
	src/transport/transport_unix.c
)
set_target_properties(packetcore PROPERTIES OUTPUT_NAME packetcore)
target_include_directories(packetcore PUBLIC include)
kilonode_target_defaults(packetcore)

add_executable(kilonode src/tools/kilonode.c)
target_link_libraries(kilonode PRIVATE packetcore)
kilonode_target_defaults(kilonode)

add_executable(kilonode-monitor src/tools/kilonode-monitor.c)
target_link_libraries(kilonode-monitor PRIVATE packetcore)
kilonode_target_defaults(kilonode-monitor)

add_executable(kilonode-msg src/tools/kilonode-msg.c)
target_link_libraries(kilonode-msg PRIVATE packetcore)
kilonode_target_defaults(kilonode-msg)

add_executable(kilonode-user src/tools/kilonode-user.c)
target_link_libraries(kilonode-user PRIVATE packetcore)
kilonode_target_defaults(kilonode-user)

add_executable(kilonode-store src/tools/kilonode-store.c)
target_link_libraries(kilonode-store PRIVATE packetcore)
kilonode_target_defaults(kilonode-store)

add_executable(kilonode-compat src/tools/kilonode-compat.c)
target_link_libraries(kilonode-compat PRIVATE packetcore)
kilonode_target_defaults(kilonode-compat)

add_executable(kilonoded src/tools/kilonoded.c)
target_link_libraries(kilonoded PRIVATE packetcore)
kilonode_target_defaults(kilonoded)

add_executable(kilonodectl src/tools/kilonodectl.c)
target_link_libraries(kilonodectl PRIVATE packetcore)
kilonode_target_defaults(kilonodectl)

kilonode_install_all()

enable_testing()

add_executable(test_buffer tests/test_buffer.c)
target_link_libraries(test_buffer PRIVATE packetcore)
kilonode_target_defaults(test_buffer)
add_test(NAME buffer COMMAND test_buffer)

add_executable(test_ax25 tests/test_ax25.c)
target_link_libraries(test_ax25 PRIVATE packetcore)
kilonode_target_defaults(test_ax25)
add_test(NAME ax25 COMMAND test_ax25)

add_executable(test_ax25_action tests/test_ax25_action.c)
target_link_libraries(test_ax25_action PRIVATE packetcore)
kilonode_target_defaults(test_ax25_action)
add_test(NAME ax25_action COMMAND test_ax25_action)

add_executable(test_ax25_action_mapper tests/test_ax25_action_mapper.c)
target_link_libraries(test_ax25_action_mapper PRIVATE packetcore)
kilonode_target_defaults(test_ax25_action_mapper)
add_test(NAME ax25_action_mapper COMMAND test_ax25_action_mapper)

add_executable(test_ax25_control tests/test_ax25_control.c)
target_link_libraries(test_ax25_control PRIVATE packetcore)
kilonode_target_defaults(test_ax25_control)
add_test(NAME ax25_control COMMAND test_ax25_control)

add_executable(test_ax25_control_plane tests/test_ax25_control_plane.c)
target_link_libraries(test_ax25_control_plane PRIVATE packetcore)
kilonode_target_defaults(test_ax25_control_plane)
add_test(NAME ax25_control_plane COMMAND test_ax25_control_plane)

add_executable(test_ax25_live_diag tests/test_ax25_live_diag.c)
target_link_libraries(test_ax25_live_diag PRIVATE packetcore)
kilonode_target_defaults(test_ax25_live_diag)
add_test(NAME ax25_live_diag COMMAND test_ax25_live_diag)

add_executable(test_ax25_connection tests/test_ax25_connection.c)
target_link_libraries(test_ax25_connection PRIVATE packetcore)
kilonode_target_defaults(test_ax25_connection)
add_test(NAME ax25_connection COMMAND test_ax25_connection)

add_executable(test_ax25_connection_diag tests/test_ax25_connection_diag.c)
target_link_libraries(test_ax25_connection_diag PRIVATE packetcore)
kilonode_target_defaults(test_ax25_connection_diag)
add_test(NAME ax25_connection_diag COMMAND test_ax25_connection_diag)

add_executable(test_ax25_connection_event tests/test_ax25_connection_event.c)
target_link_libraries(test_ax25_connection_event PRIVATE packetcore)
kilonode_target_defaults(test_ax25_connection_event)
add_test(NAME ax25_connection_event COMMAND test_ax25_connection_event)

add_executable(test_ax25_connection_key tests/test_ax25_connection_key.c)
target_link_libraries(test_ax25_connection_key PRIVATE packetcore)
kilonode_target_defaults(test_ax25_connection_key)
add_test(NAME ax25_connection_key COMMAND test_ax25_connection_key)

add_executable(test_ax25_connection_table tests/test_ax25_connection_table.c)
target_link_libraries(test_ax25_connection_table PRIVATE packetcore)
kilonode_target_defaults(test_ax25_connection_table)
add_test(NAME ax25_connection_table COMMAND test_ax25_connection_table)

add_executable(test_ax25_frame_builder tests/test_ax25_frame_builder.c)
target_link_libraries(test_ax25_frame_builder PRIVATE packetcore)
kilonode_target_defaults(test_ax25_frame_builder)
add_test(NAME ax25_frame_builder COMMAND test_ax25_frame_builder)

add_executable(test_ax25_frame_plan tests/test_ax25_frame_plan.c)
target_link_libraries(test_ax25_frame_plan PRIVATE packetcore)
kilonode_target_defaults(test_ax25_frame_plan)
add_test(NAME ax25_frame_plan COMMAND test_ax25_frame_plan)

add_executable(test_ax25_params tests/test_ax25_params.c)
target_link_libraries(test_ax25_params PRIVATE packetcore)
kilonode_target_defaults(test_ax25_params)
add_test(NAME ax25_params COMMAND test_ax25_params)

add_executable(test_ax25_retry tests/test_ax25_retry.c)
target_link_libraries(test_ax25_retry PRIVATE packetcore)
kilonode_target_defaults(test_ax25_retry)
add_test(NAME ax25_retry COMMAND test_ax25_retry)

add_executable(test_ax25_rx_feed tests/test_ax25_rx_feed.c)
target_link_libraries(test_ax25_rx_feed PRIVATE packetcore)
kilonode_target_defaults(test_ax25_rx_feed)
add_test(NAME ax25_rx_feed COMMAND test_ax25_rx_feed)

add_executable(test_ax25_runtime tests/test_ax25_runtime.c)
target_link_libraries(test_ax25_runtime PRIVATE packetcore)
kilonode_target_defaults(test_ax25_runtime)
add_test(NAME ax25_runtime COMMAND test_ax25_runtime)

add_executable(test_ax25_sequence tests/test_ax25_sequence.c)
target_link_libraries(test_ax25_sequence PRIVATE packetcore)
kilonode_target_defaults(test_ax25_sequence)
add_test(NAME ax25_sequence COMMAND test_ax25_sequence)

add_executable(test_ax25_scheduler tests/test_ax25_scheduler.c)
target_link_libraries(test_ax25_scheduler PRIVATE packetcore)
kilonode_target_defaults(test_ax25_scheduler)
add_test(NAME ax25_scheduler COMMAND test_ax25_scheduler)

add_executable(test_ax25_state tests/test_ax25_state.c)
target_link_libraries(test_ax25_state PRIVATE packetcore)
kilonode_target_defaults(test_ax25_state)
add_test(NAME ax25_state COMMAND test_ax25_state)

add_executable(test_ax25_timer tests/test_ax25_timer.c)
target_link_libraries(test_ax25_timer PRIVATE packetcore)
kilonode_target_defaults(test_ax25_timer)
add_test(NAME ax25_timer COMMAND test_ax25_timer)

add_executable(test_ax25_timer_queue tests/test_ax25_timer_queue.c)
target_link_libraries(test_ax25_timer_queue PRIVATE packetcore)
kilonode_target_defaults(test_ax25_timer_queue)
add_test(NAME ax25_timer_queue COMMAND test_ax25_timer_queue)

add_executable(test_callsign tests/test_callsign.c)
target_link_libraries(test_callsign PRIVATE packetcore)
kilonode_target_defaults(test_callsign)
add_test(NAME callsign COMMAND test_callsign)

add_executable(test_fx25 tests/test_fx25.c)
target_link_libraries(test_fx25 PRIVATE packetcore)
kilonode_target_defaults(test_fx25)
add_test(NAME fx25 COMMAND test_fx25)

add_executable(test_fx25_params tests/test_fx25_params.c)
target_link_libraries(test_fx25_params PRIVATE packetcore)
kilonode_target_defaults(test_fx25_params)
add_test(NAME fx25_params COMMAND test_fx25_params)

add_executable(test_config tests/test_config.c)
target_link_libraries(test_config PRIVATE packetcore)
kilonode_target_defaults(test_config)
add_test(NAME config COMMAND test_config)

add_executable(test_access_policy tests/test_access_policy.c)
target_link_libraries(test_access_policy PRIVATE packetcore)
kilonode_target_defaults(test_access_policy)
add_test(NAME access_policy COMMAND test_access_policy)

add_executable(test_control_protocol tests/test_control_protocol.c)
target_link_libraries(test_control_protocol PRIVATE packetcore)
kilonode_target_defaults(test_control_protocol)
add_test(NAME control_protocol COMMAND test_control_protocol)

add_executable(test_control_socket tests/test_control_socket.c)
target_link_libraries(test_control_socket PRIVATE packetcore)
kilonode_target_defaults(test_control_socket)
add_test(NAME control_socket COMMAND test_control_socket)

add_executable(test_daemon_ax25 tests/test_daemon_ax25.c)
target_link_libraries(test_daemon_ax25 PRIVATE packetcore)
kilonode_target_defaults(test_daemon_ax25)
add_test(NAME daemon_ax25 COMMAND test_daemon_ax25)

add_executable(test_compat_transcript tests/test_compat_transcript.c)
target_link_libraries(test_compat_transcript PRIVATE packetcore)
kilonode_target_defaults(test_compat_transcript)
add_test(NAME compat_transcript COMMAND test_compat_transcript)

add_executable(compat_process_helper tests/compat_process_helper.c)
kilonode_target_defaults(compat_process_helper)

add_executable(test_compat_observe tests/test_compat_observe.c)
target_link_libraries(test_compat_observe PRIVATE packetcore)
kilonode_target_defaults(test_compat_observe)
add_test(NAME compat_observe COMMAND test_compat_observe)

add_executable(test_compat_observation_pack
	tests/test_compat_observation_pack.c)
target_link_libraries(test_compat_observation_pack PRIVATE packetcore)
kilonode_target_defaults(test_compat_observation_pack)
add_test(NAME compat_observation_pack COMMAND test_compat_observation_pack)

add_executable(test_compat_coverage tests/test_compat_coverage.c)
target_link_libraries(test_compat_coverage PRIVATE packetcore)
kilonode_target_defaults(test_compat_coverage)
add_test(NAME compat_coverage COMMAND test_compat_coverage)

add_executable(test_compat_requirements tests/test_compat_requirements.c)
target_link_libraries(test_compat_requirements PRIVATE packetcore)
kilonode_target_defaults(test_compat_requirements)
add_test(NAME compat_requirements COMMAND test_compat_requirements)

add_executable(test_compat_command_profile
	tests/test_compat_command_profile.c)
target_link_libraries(test_compat_command_profile PRIVATE packetcore)
kilonode_target_defaults(test_compat_command_profile)
add_test(NAME compat_command_profile COMMAND test_compat_command_profile)

add_executable(test_compat_risk tests/test_compat_risk.c)
target_link_libraries(test_compat_risk PRIVATE packetcore)
kilonode_target_defaults(test_compat_risk)
add_test(NAME compat_risk COMMAND test_compat_risk)

add_executable(test_compat_process tests/test_compat_process.c)
target_link_libraries(test_compat_process PRIVATE packetcore)
kilonode_target_defaults(test_compat_process)
add_test(NAME compat_process COMMAND test_compat_process)

add_executable(test_compat_session tests/test_compat_session.c)
target_link_libraries(test_compat_session PRIVATE packetcore)
kilonode_target_defaults(test_compat_session)
add_test(NAME compat_session COMMAND test_compat_session)

add_executable(test_compat_capture tests/test_compat_capture.c)
target_link_libraries(test_compat_capture PRIVATE packetcore)
kilonode_target_defaults(test_compat_capture)
add_test(NAME compat_capture COMMAND test_compat_capture)

add_executable(test_compat_packet_capture tests/test_compat_packet_capture.c)
target_link_libraries(test_compat_packet_capture PRIVATE packetcore)
kilonode_target_defaults(test_compat_packet_capture)
add_test(NAME compat_packet_capture COMMAND test_compat_packet_capture)

add_executable(test_compat_kiss_capture tests/test_compat_kiss_capture.c)
target_link_libraries(test_compat_kiss_capture PRIVATE packetcore)
kilonode_target_defaults(test_compat_kiss_capture)
add_test(NAME compat_kiss_capture COMMAND test_compat_kiss_capture)

add_executable(test_compat_axip_capture tests/test_compat_axip_capture.c)
target_link_libraries(test_compat_axip_capture PRIVATE packetcore)
kilonode_target_defaults(test_compat_axip_capture)
add_test(NAME compat_axip_capture COMMAND test_compat_axip_capture)

add_executable(test_compat_capture_report tests/test_compat_capture_report.c)
target_link_libraries(test_compat_capture_report PRIVATE packetcore)
kilonode_target_defaults(test_compat_capture_report)
add_test(NAME compat_capture_report COMMAND test_compat_capture_report)

add_executable(test_compat_bench_pack tests/test_compat_bench_pack.c)
target_link_libraries(test_compat_bench_pack PRIVATE packetcore)
kilonode_target_defaults(test_compat_bench_pack)
add_test(NAME compat_bench_pack COMMAND test_compat_bench_pack)

add_executable(test_bench_ax25_diag_replay
	tests/test_bench_ax25_diag_replay.c)
target_link_libraries(test_bench_ax25_diag_replay PRIVATE packetcore)
kilonode_target_defaults(test_bench_ax25_diag_replay)
add_test(NAME bench_ax25_diag_replay COMMAND test_bench_ax25_diag_replay)

add_executable(test_bench_replay tests/test_bench_replay.c)
target_link_libraries(test_bench_replay PRIVATE packetcore)
kilonode_target_defaults(test_bench_replay)
add_test(NAME bench_replay COMMAND test_bench_replay)

add_executable(test_bench_replay_report tests/test_bench_replay_report.c)
target_link_libraries(test_bench_replay_report PRIVATE packetcore)
kilonode_target_defaults(test_bench_replay_report)
add_test(NAME bench_replay_report COMMAND test_bench_replay_report)

add_executable(test_manual_capture_workspace
	tests/test_manual_capture_workspace.c)
target_link_libraries(test_manual_capture_workspace PRIVATE packetcore)
kilonode_target_defaults(test_manual_capture_workspace)
add_test(NAME manual_capture_workspace COMMAND test_manual_capture_workspace)

add_executable(test_manual_capture_index tests/test_manual_capture_index.c)
target_link_libraries(test_manual_capture_index PRIVATE packetcore)
kilonode_target_defaults(test_manual_capture_index)
add_test(NAME manual_capture_index COMMAND test_manual_capture_index)

add_executable(test_manual_capture_replay tests/test_manual_capture_replay.c)
target_link_libraries(test_manual_capture_replay PRIVATE packetcore)
kilonode_target_defaults(test_manual_capture_replay)
add_test(NAME manual_capture_replay COMMAND test_manual_capture_replay)

add_executable(test_manual_capture_report tests/test_manual_capture_report.c)
target_link_libraries(test_manual_capture_report PRIVATE packetcore)
kilonode_target_defaults(test_manual_capture_report)
add_test(NAME manual_capture_report COMMAND test_manual_capture_report)

add_executable(test_compat_replay tests/test_compat_replay.c)
target_link_libraries(test_compat_replay PRIVATE packetcore)
kilonode_target_defaults(test_compat_replay)
add_test(NAME compat_replay COMMAND test_compat_replay)

add_executable(test_compat_report tests/test_compat_report.c)
target_link_libraries(test_compat_report PRIVATE packetcore)
kilonode_target_defaults(test_compat_report)
add_test(NAME compat_report COMMAND test_compat_report)

add_executable(test_daemon_config tests/test_daemon_config.c)
target_link_libraries(test_daemon_config PRIVATE packetcore)
kilonode_target_defaults(test_daemon_config)
add_test(NAME daemon_config COMMAND test_daemon_config)

add_executable(test_daemon_heard tests/test_daemon_heard.c)
target_link_libraries(test_daemon_heard PRIVATE packetcore)
kilonode_target_defaults(test_daemon_heard)
add_test(NAME daemon_heard COMMAND test_daemon_heard)

add_executable(test_heard tests/test_heard.c)
target_link_libraries(test_heard PRIVATE packetcore)
kilonode_target_defaults(test_heard)
add_test(NAME heard COMMAND test_heard)

add_executable(test_rf_command tests/test_rf_command.c)
target_link_libraries(test_rf_command PRIVATE packetcore)
kilonode_target_defaults(test_rf_command)
add_test(NAME rf_command COMMAND test_rf_command)

add_executable(test_rf_command_queue tests/test_rf_command_queue.c)
target_link_libraries(test_rf_command_queue PRIVATE packetcore)
kilonode_target_defaults(test_rf_command_queue)
add_test(NAME rf_command_queue COMMAND test_rf_command_queue)

add_executable(test_rf_abuse tests/test_rf_abuse.c)
target_link_libraries(test_rf_abuse PRIVATE packetcore)
kilonode_target_defaults(test_rf_abuse)
add_test(NAME rf_abuse COMMAND test_rf_abuse)

add_executable(test_rf_ignore tests/test_rf_ignore.c)
target_link_libraries(test_rf_ignore PRIVATE packetcore)
kilonode_target_defaults(test_rf_ignore)
add_test(NAME rf_ignore COMMAND test_rf_ignore)

add_executable(test_rf_reply tests/test_rf_reply.c)
target_link_libraries(test_rf_reply PRIVATE packetcore)
kilonode_target_defaults(test_rf_reply)
add_test(NAME rf_reply COMMAND test_rf_reply)

add_executable(test_rx_event tests/test_rx_event.c)
target_link_libraries(test_rx_event PRIVATE packetcore)
kilonode_target_defaults(test_rx_event)
add_test(NAME rx_event COMMAND test_rx_event)

add_executable(test_rx_queue tests/test_rx_queue.c)
target_link_libraries(test_rx_queue PRIVATE packetcore)
kilonode_target_defaults(test_rx_queue)
add_test(NAME rx_queue COMMAND test_rx_queue)

add_executable(test_rx_session tests/test_rx_session.c)
target_link_libraries(test_rx_session PRIVATE packetcore)
kilonode_target_defaults(test_rx_session)
add_test(NAME rx_session COMMAND test_rx_session)

add_executable(test_daemon_rx tests/test_daemon_rx.c)
target_link_libraries(test_daemon_rx PRIVATE packetcore)
kilonode_target_defaults(test_daemon_rx)
add_test(NAME daemon_rx COMMAND test_daemon_rx)

add_executable(test_daemon_rf_command tests/test_daemon_rf_command.c)
target_link_libraries(test_daemon_rf_command PRIVATE packetcore)
kilonode_target_defaults(test_daemon_rf_command)
add_test(NAME daemon_rf_command COMMAND test_daemon_rf_command)

add_executable(test_node_command tests/test_node_command.c)
target_link_libraries(test_node_command PRIVATE packetcore)
kilonode_target_defaults(test_node_command)
add_test(NAME node_command COMMAND test_node_command)

add_executable(test_node_command_profile tests/test_node_command_profile.c)
target_link_libraries(test_node_command_profile PRIVATE packetcore)
kilonode_target_defaults(test_node_command_profile)
add_test(NAME node_command_profile COMMAND test_node_command_profile)

add_executable(test_node_command_dispatch tests/test_node_command_dispatch.c)
target_link_libraries(test_node_command_dispatch PRIVATE packetcore)
kilonode_target_defaults(test_node_command_dispatch)
add_test(NAME node_command_dispatch COMMAND test_node_command_dispatch)

add_executable(test_tx_frame tests/test_tx_frame.c)
target_link_libraries(test_tx_frame PRIVATE packetcore)
kilonode_target_defaults(test_tx_frame)
add_test(NAME tx_frame COMMAND test_tx_frame)

add_executable(test_tx_dry_run tests/test_tx_dry_run.c)
target_link_libraries(test_tx_dry_run PRIVATE packetcore)
kilonode_target_defaults(test_tx_dry_run)
add_test(NAME tx_dry_run COMMAND test_tx_dry_run)

add_executable(test_tx_dispatch tests/test_tx_dispatch.c)
target_link_libraries(test_tx_dispatch PRIVATE packetcore)
kilonode_target_defaults(test_tx_dispatch)
add_test(NAME tx_dispatch COMMAND test_tx_dispatch)

add_executable(test_tx_queue tests/test_tx_queue.c)
target_link_libraries(test_tx_queue PRIVATE packetcore)
kilonode_target_defaults(test_tx_queue)
add_test(NAME tx_queue COMMAND test_tx_queue)

add_executable(test_tx_policy tests/test_tx_policy.c)
target_link_libraries(test_tx_policy PRIVATE packetcore)
kilonode_target_defaults(test_tx_policy)
add_test(NAME tx_policy COMMAND test_tx_policy)

add_executable(test_tx_transport_gate tests/test_tx_transport_gate.c)
target_link_libraries(test_tx_transport_gate PRIVATE packetcore)
kilonode_target_defaults(test_tx_transport_gate)
add_test(NAME tx_transport_gate COMMAND test_tx_transport_gate)

add_executable(test_daemon_tx tests/test_daemon_tx.c)
target_link_libraries(test_daemon_tx PRIVATE packetcore)
kilonode_target_defaults(test_daemon_tx)
add_test(NAME daemon_tx COMMAND test_daemon_tx)

add_executable(test_kiss tests/test_kiss.c)
target_link_libraries(test_kiss PRIVATE packetcore)
kilonode_target_defaults(test_kiss)
add_test(NAME kiss COMMAND test_kiss)

add_executable(test_kiss_stream tests/test_kiss_stream.c)
target_link_libraries(test_kiss_stream PRIVATE packetcore)
kilonode_target_defaults(test_kiss_stream)
add_test(NAME kiss_stream COMMAND test_kiss_stream)

add_executable(test_monitor tests/test_monitor.c)
target_link_libraries(test_monitor PRIVATE packetcore)
kilonode_target_defaults(test_monitor)
add_test(NAME monitor COMMAND test_monitor)

add_executable(test_message tests/test_message.c)
target_link_libraries(test_message PRIVATE packetcore)
kilonode_target_defaults(test_message)
add_test(NAME message COMMAND test_message)

add_executable(test_bbs_area tests/test_bbs_area.c)
target_link_libraries(test_bbs_area PRIVATE packetcore)
kilonode_target_defaults(test_bbs_area)
add_test(NAME bbs_area COMMAND test_bbs_area)

add_executable(test_bbs_control tests/test_bbs_control.c)
target_link_libraries(test_bbs_control PRIVATE packetcore)
kilonode_target_defaults(test_bbs_control)
add_test(NAME bbs_control COMMAND test_bbs_control)

add_executable(test_bbs_read_state tests/test_bbs_read_state.c)
target_link_libraries(test_bbs_read_state PRIVATE packetcore)
kilonode_target_defaults(test_bbs_read_state)
add_test(NAME bbs_read_state COMMAND test_bbs_read_state)

add_executable(test_bbs_user tests/test_bbs_user.c)
target_link_libraries(test_bbs_user PRIVATE packetcore)
kilonode_target_defaults(test_bbs_user)
add_test(NAME bbs_user COMMAND test_bbs_user)

add_executable(test_bbs_store_lock tests/test_bbs_store_lock.c)
target_link_libraries(test_bbs_store_lock PRIVATE packetcore)
kilonode_target_defaults(test_bbs_store_lock)
add_test(NAME bbs_store_lock COMMAND test_bbs_store_lock)

add_executable(test_bbs_store_maintenance tests/test_bbs_store_maintenance.c)
target_link_libraries(test_bbs_store_maintenance PRIVATE packetcore)
kilonode_target_defaults(test_bbs_store_maintenance)
add_test(NAME bbs_store_maintenance COMMAND test_bbs_store_maintenance)

add_executable(test_message_index tests/test_message_index.c)
target_link_libraries(test_message_index PRIVATE packetcore)
kilonode_target_defaults(test_message_index)
add_test(NAME message_index COMMAND test_message_index)

add_executable(test_message_store tests/test_message_store.c)
target_link_libraries(test_message_store PRIVATE packetcore)
kilonode_target_defaults(test_message_store)
add_test(NAME message_store COMMAND test_message_store)

add_executable(test_bbs_shell tests/test_bbs_shell.c)
target_link_libraries(test_bbs_shell PRIVATE packetcore)
kilonode_target_defaults(test_bbs_shell)
add_test(NAME bbs_shell COMMAND test_bbs_shell)

add_executable(test_bbs_shell_store tests/test_bbs_shell_store.c)
target_link_libraries(test_bbs_shell_store PRIVATE packetcore)
kilonode_target_defaults(test_bbs_shell_store)
add_test(NAME bbs_shell_store COMMAND test_bbs_shell_store)

add_executable(test_node_shell tests/test_node_shell.c)
target_link_libraries(test_node_shell PRIVATE packetcore)
kilonode_target_defaults(test_node_shell)
add_test(NAME node_shell COMMAND test_node_shell)

add_executable(test_node_shell_socket tests/test_node_shell_socket.c)
target_link_libraries(test_node_shell_socket PRIVATE packetcore)
kilonode_target_defaults(test_node_shell_socket)
add_test(NAME node_shell_socket COMMAND test_node_shell_socket)

add_executable(test_stats tests/test_stats.c)
target_link_libraries(test_stats PRIVATE packetcore)
kilonode_target_defaults(test_stats)
add_test(NAME stats COMMAND test_stats)

add_executable(test_session_limits tests/test_session_limits.c)
target_link_libraries(test_session_limits PRIVATE packetcore)
kilonode_target_defaults(test_session_limits)
add_test(NAME session_limits COMMAND test_session_limits)

add_executable(test_transport tests/test_transport.c)
target_link_libraries(test_transport PRIVATE packetcore)
kilonode_target_defaults(test_transport)
add_test(NAME transport COMMAND test_transport)

add_executable(test_transport_memory tests/test_transport_memory.c)
target_link_libraries(test_transport_memory PRIVATE packetcore)
kilonode_target_defaults(test_transport_memory)
add_test(NAME transport_memory COMMAND test_transport_memory)

add_executable(test_transport_serial tests/test_transport_serial.c)
target_link_libraries(test_transport_serial PRIVATE packetcore)
kilonode_target_defaults(test_transport_serial)
add_test(NAME transport_serial COMMAND test_transport_serial)

add_executable(test_transport_pty tests/test_transport_pty.c)
target_link_libraries(test_transport_pty PRIVATE packetcore)
kilonode_target_defaults(test_transport_pty)
add_test(NAME transport_pty COMMAND test_transport_pty)

add_executable(test_transport_unix tests/test_transport_unix.c)
target_link_libraries(test_transport_unix PRIVATE packetcore)
kilonode_target_defaults(test_transport_unix)
add_test(NAME transport_unix COMMAND test_transport_unix)
