/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_timer_replay_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_timer_replay_report.h"

static enum kn_ax25_timer_replay_report_error append_text(
	char *, size_t, size_t *, const char *);

static enum kn_ax25_timer_replay_report_error
append_text(char *buf, size_t bufsiz, size_t *used, const char *text)
{
	size_t len;

	if (buf == NULL || used == NULL || text == NULL || *used >= bufsiz)
		return KN_AX25_TIMER_REPLAY_REPORT_ERR_INVALID_ARGUMENT;

	len = strlen(text);
	if (len >= bufsiz - *used)
		return KN_AX25_TIMER_REPLAY_REPORT_ERR_BUFFER;
	memcpy(buf + *used, text, len + 1);
	*used += len;
	return KN_AX25_TIMER_REPLAY_REPORT_OK;
}

enum kn_ax25_timer_replay_report_error
kn_ax25_timer_replay_report_format(
	const struct kn_ax25_timer_replay_result *result, char *buf,
	size_t bufsiz)
{
	char line[256];
	size_t used;
	size_t i;
	int needed;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_TIMER_REPLAY_REPORT_ERR_INVALID_ARGUMENT;

	buf[0] = '\0';
	used = 0;
	needed = snprintf(line, sizeof(line),
	    "AX25-TIMER-REPLAY name=%s result=%s state=%s retry=%u "
	    "connections=%llu plans=%llu tx_writes=%llu mismatches=%llu\n",
	    result->name, result->pass != 0 ? "pass" : "fail",
	    kn_ax25_connection_state_name(result->final_state),
	    (unsigned int)result->retry_count,
	    (unsigned long long)result->connection_count,
	    (unsigned long long)result->frame_plans_seen,
	    (unsigned long long)result->tx_writes_observed,
	    (unsigned long long)result->mismatch_count);
	if (needed < 0 || (size_t)needed >= sizeof(line))
		return KN_AX25_TIMER_REPLAY_REPORT_ERR_BUFFER;
	if (append_text(buf, bufsiz, &used, line) !=
	    KN_AX25_TIMER_REPLAY_REPORT_OK)
		return KN_AX25_TIMER_REPLAY_REPORT_ERR_BUFFER;

	needed = snprintf(line, sizeof(line),
	    "AX25-TIMER timers t1=%s t2=%s t3=%s started=%llu "
	    "stopped=%llu expired=%llu retries=%llu exhausted=%llu\n",
	    result->t1_running != 0 ? "running" : "stopped",
	    result->t2_running != 0 ? "running" : "stopped",
	    result->t3_running != 0 ? "running" : "stopped",
	    (unsigned long long)result->scheduler_counters.timers_started,
	    (unsigned long long)result->scheduler_counters.timers_stopped,
	    (unsigned long long)result->scheduler_counters.timers_expired,
	    (unsigned long long)result->scheduler_counters.retries_incremented,
	    (unsigned long long)result->scheduler_counters.retries_exhausted);
	if (needed < 0 || (size_t)needed >= sizeof(line))
		return KN_AX25_TIMER_REPLAY_REPORT_ERR_BUFFER;
	if (append_text(buf, bufsiz, &used, line) !=
	    KN_AX25_TIMER_REPLAY_REPORT_OK)
		return KN_AX25_TIMER_REPLAY_REPORT_ERR_BUFFER;

	for (i = 0; i < result->mismatch_count; i++) {
		needed = snprintf(line, sizeof(line),
		    "AX25-TIMER-MISMATCH line=%llu detail=%s\n",
		    (unsigned long long)result->mismatches[i].line,
		    result->mismatches[i].detail);
		if (needed < 0 || (size_t)needed >= sizeof(line))
			return KN_AX25_TIMER_REPLAY_REPORT_ERR_BUFFER;
		if (append_text(buf, bufsiz, &used, line) !=
		    KN_AX25_TIMER_REPLAY_REPORT_OK)
			return KN_AX25_TIMER_REPLAY_REPORT_ERR_BUFFER;
	}

	return KN_AX25_TIMER_REPLAY_REPORT_OK;
}
