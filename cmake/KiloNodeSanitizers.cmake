# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/cmake/KiloNodeSanitizers.cmake

option(KILONODE_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(KILONODE_SANITIZE_ADDRESS "Enable AddressSanitizer" OFF)
option(KILONODE_SANITIZE_UNDEFINED "Enable UndefinedBehaviorSanitizer" OFF)
option(KILONODE_SANITIZE_LEAK "Enable LeakSanitizer where supported" OFF)

function(kilonode_apply_sanitizers target_name)
	set(sanitize_flags "")

	if(KILONODE_SANITIZE)
		set(KILONODE_SANITIZE_ADDRESS ON)
		set(KILONODE_SANITIZE_UNDEFINED ON)
	endif()

	if(KILONODE_SANITIZE_ADDRESS)
		list(APPEND sanitize_flags "address")
	endif()

	if(KILONODE_SANITIZE_UNDEFINED)
		list(APPEND sanitize_flags "undefined")
	endif()

	if(KILONODE_SANITIZE_LEAK)
		list(APPEND sanitize_flags "leak")
	endif()

	if(NOT sanitize_flags)
		return()
	endif()

	if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
		message(FATAL_ERROR "KiloNode sanitizers require GCC or Clang")
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		message(WARNING "KiloNode sanitizers are enabled for a Release build")
	endif()

	list(JOIN sanitize_flags "," sanitize_list)
	target_compile_options(${target_name} PRIVATE
		-fsanitize=${sanitize_list}
		-fno-omit-frame-pointer
	)
	target_link_options(${target_name} PRIVATE -fsanitize=${sanitize_list})
endfunction()
