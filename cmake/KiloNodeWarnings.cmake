# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/cmake/KiloNodeWarnings.cmake

option(KILONODE_WARNINGS_AS_ERRORS "Treat supported warnings as errors" OFF)

function(kilonode_apply_warnings target_name)
	if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
		target_compile_options(${target_name} PRIVATE
			-Wall
			-Wextra
			-Wpedantic
			-Wconversion
			-Wshadow
			-Wstrict-prototypes
			-Wmissing-prototypes
			-Wold-style-definition
		)

		if(KILONODE_WARNINGS_AS_ERRORS)
			target_compile_options(${target_name} PRIVATE -Werror)
		endif()
	else()
		message(STATUS "KiloNode: no warning profile for ${CMAKE_C_COMPILER_ID}")
	endif()
endfunction()
