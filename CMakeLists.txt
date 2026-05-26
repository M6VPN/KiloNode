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
	src/core/buffer.c
	src/core/callsign.c
	src/proto/ax25.c
	src/proto/kiss.c
)
set_target_properties(packetcore PROPERTIES OUTPUT_NAME packetcore)
target_include_directories(packetcore PUBLIC include)
kilonode_target_defaults(packetcore)

add_executable(kilonode src/tools/kilonode.c)
target_link_libraries(kilonode PRIVATE packetcore)
kilonode_target_defaults(kilonode)

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

add_executable(test_kiss tests/test_kiss.c)
target_link_libraries(test_kiss PRIVATE packetcore)
kilonode_target_defaults(test_kiss)
add_test(NAME kiss COMMAND test_kiss)
