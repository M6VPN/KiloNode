/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/daemon/daemon_ax25_scheduler.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/daemon_ax25_scheduler.h"

enum kn_daemon_ax25_scheduler_error
kn_daemon_ax25_scheduler_poll(struct kn_ax25_runtime *runtime,
	uint64_t now_ms)
{
	if (runtime == NULL)
		return KN_DAEMON_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;
	if (runtime->live_scheduler.policy.enabled == 0)
		return KN_DAEMON_AX25_SCHEDULER_OK;
	if (runtime->smoke_options.enabled != 0) {
		if (kn_ax25_scheduler_smoke_poll(runtime, now_ms) !=
		    KN_AX25_SCHEDULER_SMOKE_OK)
			return KN_DAEMON_AX25_SCHEDULER_ERR_RUNTIME;
		return KN_DAEMON_AX25_SCHEDULER_OK;
	}
	(void)kn_ax25_live_scheduler_poll(runtime, now_ms);
	return KN_DAEMON_AX25_SCHEDULER_OK;
}
