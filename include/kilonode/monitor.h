/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/monitor.h */

#ifndef KILONODE_MONITOR_H
#define KILONODE_MONITOR_H

#include <sys/types.h>

#include <stdint.h>

enum kn_monitor_error {
	KN_MONITOR_OK = 0,
	KN_MONITOR_ERR_INVALID_ARGUMENT,
	KN_MONITOR_ERR_NO_SPACE
};

enum kn_monitor_error kn_monitor_format_kiss(char *, size_t, uint8_t, uint8_t,
	const uint8_t *, size_t);

#endif
