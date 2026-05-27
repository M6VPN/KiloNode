# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/cmake/KiloNodeInstall.cmake

set(KILONODE_DEFAULT_CONFIG_PATH
	"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/kilonode/kilonode.conf"
	CACHE STRING "Default installed KiloNode config path")
set(KILONODE_DEFAULT_RUNTIME_DIR
	"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALSTATEDIR}/run/kilonode"
	CACHE STRING "Default KiloNode runtime directory")
set(KILONODE_DEFAULT_DATA_DIR
	"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALSTATEDIR}/lib/kilonode"
	CACHE STRING "Default KiloNode data directory")
option(KILONODE_INSTALL_SERVICE_EXAMPLES
	"Install service manager examples under documentation" ON)

function(kilonode_install_targets)
	install(TARGETS
		kilonode
		kilonode-compat
		kilonode-monitor
		kilonode-msg
		kilonode-store
		kilonode-user
		kilonodectl
		kilonoded
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endfunction()

function(kilonode_install_examples)
	install(DIRECTORY packaging/examples/
		DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
		FILES_MATCHING PATTERN "*.conf" PATTERN "README.md"
	)
endfunction()

function(kilonode_install_manpages)
	install(FILES
		docs/man/kilonodectl.1
		docs/man/kilonode-compat.1
		docs/man/kilonode-monitor.1
		docs/man/kilonode-msg.1
		docs/man/kilonode-store.1
		docs/man/kilonode-user.1
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
	)
	install(FILES docs/man/kilonoded.8
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man8
	)
endfunction()

function(kilonode_install_service_examples)
	if(KILONODE_INSTALL_SERVICE_EXAMPLES)
		install(DIRECTORY packaging/
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/packaging
			PATTERN "examples" EXCLUDE
		)
	endif()
endfunction()

function(kilonode_install_all)
	kilonode_install_targets()
	kilonode_install_examples()
	kilonode_install_manpages()
	kilonode_install_service_examples()
endfunction()
