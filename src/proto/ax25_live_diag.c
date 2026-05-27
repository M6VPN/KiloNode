/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_live_diag.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_live_diag.h"

enum kn_ax25_live_diag_error
kn_ax25_live_diag_format(const struct kn_ax25_runtime *runtime, char *buf,
	size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *source;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_LIVE_DIAG_ERR_INVALID_ARGUMENT;

	source = runtime;
	if (source == NULL) {
		kn_ax25_runtime_init(&fallback);
		source = &fallback;
	}

	needed = snprintf(buf, bufsiz,
	    "OK AX25 LIVE enabled=%s feed=%s create_connections=%s "
	    "seen=%llu accepted=%llu ignored=%llu malformed=%llu "
	    "not_relevant=%llu ui_ignored=%llu events=%llu plans=%llu "
	    "tx_writes=%llu\nEND\n",
	    source->enabled != 0 ? "true" : "false",
	    source->live.live_rx_feed != 0 ? "true" : "false",
	    source->live.live_rx_create_connections != 0 ? "true" : "false",
	    (unsigned long long)source->live_counters.frames_seen,
	    (unsigned long long)source->live_counters.frames_accepted,
	    (unsigned long long)source->live_counters.frames_ignored,
	    (unsigned long long)source->live_counters.frames_malformed,
	    (unsigned long long)source->live_counters.frames_not_relevant,
	    (unsigned long long)source->live_counters.ui_ignored,
	    (unsigned long long)source->live_counters.events_generated,
	    (unsigned long long)source->live_counters.frame_plans_retained,
	    (unsigned long long)source->live_counters.tx_queue_writes_attempted);
	if (runtime == NULL)
		kn_ax25_runtime_free(&fallback);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_LIVE_DIAG_ERR_TRUNCATED;

	return KN_AX25_LIVE_DIAG_OK;
}
