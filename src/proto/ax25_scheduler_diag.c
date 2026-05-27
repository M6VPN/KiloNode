/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_scheduler_diag.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>

#include "kilonode/ax25_scheduler_diag.h"

static enum kn_ax25_scheduler_diag_error append_format(char *, size_t,
	size_t *, const char *, ...);
static const char *diag_timer_kind_name(enum kn_ax25_timer_kind);

static enum kn_ax25_scheduler_diag_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (buf == NULL || offset == NULL || fmt == NULL || bufsiz == 0)
		return KN_AX25_SCHEDULER_DIAG_ERR_INVALID_ARGUMENT;
	if (*offset >= bufsiz)
		return KN_AX25_SCHEDULER_DIAG_ERR_BUFFER;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_AX25_SCHEDULER_DIAG_ERR_BUFFER;

	*offset += (size_t)needed;
	return KN_AX25_SCHEDULER_DIAG_OK;
}

static const char *
diag_timer_kind_name(enum kn_ax25_timer_kind kind)
{
	switch (kind) {
	case KN_AX25_TIMER_T1:
		return "T1";
	case KN_AX25_TIMER_T2:
		return "T2";
	case KN_AX25_TIMER_T3:
		return "T3";
	default:
		break;
	}

	return kn_ax25_timer_kind_name(kind);
}

enum kn_ax25_scheduler_diag_error
kn_ax25_scheduler_diag_format_counters(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_SCHEDULER_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 SCHEDULER COUNTERS cycles=%llu expired=%llu "
	    "blocked=%llu plans=%llu tx_blocked=%llu tx_writes=%llu\nEND\n",
	    (unsigned long long)rt->live_scheduler.cycles_run,
	    (unsigned long long)rt->live_scheduler.expired_processed,
	    (unsigned long long)rt->live_scheduler.expired_blocked_by_policy,
	    (unsigned long long)rt->live_scheduler.generated_frame_plans,
	    (unsigned long long)rt->live_scheduler.tx_actions_blocked,
	    (unsigned long long)rt->live_scheduler.tx_writes_attempted);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_SCHEDULER_DIAG_ERR_BUFFER;

	return KN_AX25_SCHEDULER_DIAG_OK;
}

enum kn_ax25_scheduler_diag_error
kn_ax25_scheduler_diag_format_status(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_SCHEDULER_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 SCHEDULER enabled=%s process_expired=%s "
	    "max_expired=%llu tx_actions=%s next_wakeup=%llu\nEND\n",
	    rt->live_scheduler.policy.enabled != 0 ? "true" : "false",
	    rt->live_scheduler.policy.process_expired != 0 ? "true" :
	    "false",
	    (unsigned long long)
	    rt->live_scheduler.policy.max_expired_per_cycle,
	    rt->live_scheduler.policy.tx_actions_enabled != 0 ? "true" :
	    "false",
	    (unsigned long long)rt->live_scheduler.next_wakeup_ms);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_SCHEDULER_DIAG_ERR_BUFFER;

	return KN_AX25_SCHEDULER_DIAG_OK;
}

enum kn_ax25_scheduler_diag_error
kn_ax25_scheduler_diag_format_timers(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	const struct kn_ax25_timer *timer;
	size_t i;
	size_t offset;
	enum kn_ax25_scheduler_diag_error rc;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_SCHEDULER_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK AX25 SCHEDULER TIMERS count=%llu\n",
	    (unsigned long long)rt->scheduler.queue.count);
	if (rc != KN_AX25_SCHEDULER_DIAG_OK)
		return rc;
	for (i = 0; i < rt->scheduler.queue.count; i++) {
		timer = &rt->scheduler.queue.timers[i];
		rc = append_format(buf, bufsiz, &offset,
		    "AX25 TIMER index=%llu conn=%llu kind=%s running=%s "
		    "started=%llu expires=%llu duration=%llu generation=%llu\n",
		    (unsigned long long)i,
		    (unsigned long long)timer->connection_id,
		    diag_timer_kind_name(timer->kind),
		    timer->running != 0 ? "true" : "false",
		    (unsigned long long)timer->started_at_ms,
		    (unsigned long long)timer->expires_at_ms,
		    (unsigned long long)timer->duration_ms,
		    (unsigned long long)timer->generation);
		if (rc != KN_AX25_SCHEDULER_DIAG_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}
