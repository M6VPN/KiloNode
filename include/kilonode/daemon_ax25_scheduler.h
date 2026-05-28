/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/daemon_ax25_scheduler.h */

#ifndef KILONODE_DAEMON_AX25_SCHEDULER_H
#define KILONODE_DAEMON_AX25_SCHEDULER_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_runtime.h"

enum kn_daemon_ax25_scheduler_error {
	KN_DAEMON_AX25_SCHEDULER_OK = 0,
	KN_DAEMON_AX25_SCHEDULER_ERR_INVALID_ARGUMENT,
	KN_DAEMON_AX25_SCHEDULER_ERR_RUNTIME
};

enum kn_daemon_ax25_scheduler_error kn_daemon_ax25_scheduler_poll(
	struct kn_ax25_runtime *, uint64_t);

#endif
