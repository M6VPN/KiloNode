/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/daemon.h */

#ifndef KILONODE_DAEMON_H
#define KILONODE_DAEMON_H

#include "kilonode/config.h"

enum kn_daemon_error {
	KN_DAEMON_OK = 0,
	KN_DAEMON_ERR_INVALID_ARGUMENT,
	KN_DAEMON_ERR_CONFIG,
	KN_DAEMON_ERR_TRANSPORT,
	KN_DAEMON_ERR_RUNTIME
};

enum kn_daemon_error kn_daemon_run_foreground(const struct kn_config *);

#endif
