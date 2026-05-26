# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(KiloNode VERSION 0.1.0 LANGUAGES C)

option(KILONODE_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

function(kilonode_target_defaults target_name)
	target_compile_features(${target_name} PRIVATE c_std_17)

	if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
		target_compile_options(${target_name} PRIVATE
			-Wall
			-Wextra
			-Wpedantic
			-Wconversion
			-Wshadow
			-Wstrict-prototypes
			-fstack-protector-strong
		)
	endif()

	if(KILONODE_SANITIZE)
		if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
			target_compile_options(${target_name} PRIVATE
				-fsanitize=address,undefined
				-fno-omit-frame-pointer
			)
			target_link_options(${target_name} PRIVATE
				-fsanitize=address,undefined
			)
		endif()
	endif()
endfunction()

add_library(packetcore STATIC
	src/control/control_protocol.c
	src/control/control_socket.c
	src/core/buffer.c
	src/core/callsign.c
	src/core/config.c
	src/core/heard.c
	src/core/stats.c
	src/daemon/daemon.c
	src/monitor/monitor.c
	src/node/node_shell.c
	src/proto/ax25.c
	src/proto/kiss.c
	src/proto/kiss_stream.c
	src/transport/transport.c
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

add_executable(kilonoded src/tools/kilonoded.c)
target_link_libraries(kilonoded PRIVATE packetcore)
kilonode_target_defaults(kilonoded)

add_executable(kilonodectl src/tools/kilonodectl.c)
target_link_libraries(kilonodectl PRIVATE packetcore)
kilonode_target_defaults(kilonodectl)

enable_testing()

add_executable(test_buffer tests/test_buffer.c)
target_link_libraries(test_buffer PRIVATE packetcore)
kilonode_target_defaults(test_buffer)
add_test(NAME buffer COMMAND test_buffer)

add_executable(test_ax25 tests/test_ax25.c)
target_link_libraries(test_ax25 PRIVATE packetcore)
kilonode_target_defaults(test_ax25)
add_test(NAME ax25 COMMAND test_ax25)

add_executable(test_callsign tests/test_callsign.c)
target_link_libraries(test_callsign PRIVATE packetcore)
kilonode_target_defaults(test_callsign)
add_test(NAME callsign COMMAND test_callsign)

add_executable(test_config tests/test_config.c)
target_link_libraries(test_config PRIVATE packetcore)
kilonode_target_defaults(test_config)
add_test(NAME config COMMAND test_config)

add_executable(test_control_protocol tests/test_control_protocol.c)
target_link_libraries(test_control_protocol PRIVATE packetcore)
kilonode_target_defaults(test_control_protocol)
add_test(NAME control_protocol COMMAND test_control_protocol)

add_executable(test_control_socket tests/test_control_socket.c)
target_link_libraries(test_control_socket PRIVATE packetcore)
kilonode_target_defaults(test_control_socket)
add_test(NAME control_socket COMMAND test_control_socket)

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

add_executable(test_transport tests/test_transport.c)
target_link_libraries(test_transport PRIVATE packetcore)
kilonode_target_defaults(test_transport)
add_test(NAME transport COMMAND test_transport)

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
