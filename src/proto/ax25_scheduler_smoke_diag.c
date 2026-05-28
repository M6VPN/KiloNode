/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_scheduler_smoke_diag.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_scheduler_smoke_diag.h"

enum kn_ax25_scheduler_smoke_diag_error
kn_ax25_scheduler_smoke_diag_format(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_SCHEDULER_SMOKE_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 SCHEDULER SMOKE enabled=%s create_test_connection=%s "
	    "cycles=%llu test_connections=%llu polls=%llu expired=%llu "
	    "prepared=%llu bridge_blocked=%llu tx_writes=%llu "
	    "dispatch_calls=%llu\nEND\n",
	    rt->smoke_options.enabled != 0 ? "true" : "false",
	    rt->smoke_options.create_test_connection != 0 ? "true" :
	    "false",
	    (unsigned long long)rt->smoke_counters.cycles,
	    (unsigned long long)rt->smoke_counters.test_connections_created,
	    (unsigned long long)rt->smoke_counters.scheduler_polls,
	    (unsigned long long)rt->smoke_counters.expired_processed,
	    (unsigned long long)rt->smoke_counters.prepared_frames_generated,
	    (unsigned long long)rt->smoke_counters.prepared_bridge_blocked,
	    (unsigned long long)rt->smoke_counters.tx_writes_attempted,
	    (unsigned long long)rt->smoke_counters.dispatch_calls_attempted);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_SCHEDULER_SMOKE_DIAG_ERR_BUFFER;

	return KN_AX25_SCHEDULER_SMOKE_DIAG_OK;
}
