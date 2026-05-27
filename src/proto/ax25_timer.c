/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_timer.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_timer.h"

void
kn_ax25_timer_init(struct kn_ax25_timer *timer)
{
	if (timer == NULL)
		return;

	memset(timer, 0, sizeof(*timer));
}

enum kn_ax25_timer_error
kn_ax25_timer_format(const struct kn_ax25_timer *timer, char *buf,
	size_t bufsiz)
{
	int needed;

	if (timer == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_TIMER_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "kind=%s connection=%u running=%s started=%llu expires=%llu "
	    "duration=%u generation=%u expiries=%u",
	    kn_ax25_timer_kind_name(timer->kind),
	    (unsigned int)timer->connection_id,
	    timer->running != 0 ? "true" : "false",
	    (unsigned long long)timer->started_at_ms,
	    (unsigned long long)timer->expires_at_ms,
	    (unsigned int)timer->duration_ms,
	    (unsigned int)timer->generation,
	    (unsigned int)timer->expiry_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_TIMER_ERR_BUFFER;

	return KN_AX25_TIMER_OK;
}

enum kn_ax25_connection_event
kn_ax25_timer_kind_event(enum kn_ax25_timer_kind kind)
{
	switch (kind) {
	case KN_AX25_TIMER_T1:
		return KN_AX25_CONNECTION_EVENT_TIMEOUT_T1;
	case KN_AX25_TIMER_T3:
		return KN_AX25_CONNECTION_EVENT_TIMEOUT_T3;
	case KN_AX25_TIMER_NONE:
	case KN_AX25_TIMER_T2:
	case KN_AX25_TIMER_IDLE_SESSION:
	case KN_AX25_TIMER_BUSY_CONDITION:
		return KN_AX25_CONNECTION_EVENT_NONE;
	}

	return KN_AX25_CONNECTION_EVENT_NONE;
}

const char *
kn_ax25_timer_kind_name(enum kn_ax25_timer_kind kind)
{
	switch (kind) {
	case KN_AX25_TIMER_NONE:
		return "none";
	case KN_AX25_TIMER_T1:
		return "t1";
	case KN_AX25_TIMER_T2:
		return "t2";
	case KN_AX25_TIMER_T3:
		return "t3";
	case KN_AX25_TIMER_IDLE_SESSION:
		return "idle-session";
	case KN_AX25_TIMER_BUSY_CONDITION:
		return "busy-condition";
	}

	return "unknown";
}

uint64_t
kn_ax25_timer_remaining_ms(const struct kn_ax25_timer *timer, uint64_t now_ms)
{
	if (timer == NULL || timer->running == 0)
		return 0;
	if (now_ms >= timer->expires_at_ms)
		return 0;

	return timer->expires_at_ms - now_ms;
}

enum kn_ax25_timer_error
kn_ax25_timer_restart(struct kn_ax25_timer *timer, uint64_t now_ms)
{
	enum kn_ax25_timer_kind kind;
	uint32_t connection_id;
	uint32_t duration_ms;

	if (timer == NULL)
		return KN_AX25_TIMER_ERR_INVALID_ARGUMENT;
	if (timer->duration_ms == 0)
		return KN_AX25_TIMER_ERR_INVALID_VALUE;

	kind = timer->kind;
	connection_id = timer->connection_id;
	duration_ms = timer->duration_ms;
	return kn_ax25_timer_start(timer, kind, connection_id, now_ms,
	    duration_ms);
}

enum kn_ax25_timer_error
kn_ax25_timer_start(struct kn_ax25_timer *timer,
	enum kn_ax25_timer_kind kind, uint32_t connection_id, uint64_t now_ms,
	uint32_t duration_ms)
{
	if (timer == NULL)
		return KN_AX25_TIMER_ERR_INVALID_ARGUMENT;
	if (kind == KN_AX25_TIMER_NONE)
		return KN_AX25_TIMER_ERR_INVALID_VALUE;
	if (duration_ms == 0 ||
	    duration_ms > KN_AX25_TIMER_MAX_DURATION_MS)
		return KN_AX25_TIMER_ERR_INVALID_VALUE;
	if (UINT64_MAX - now_ms < (uint64_t)duration_ms)
		return KN_AX25_TIMER_ERR_INVALID_VALUE;

	timer->kind = kind;
	timer->running = 1;
	timer->connection_id = connection_id;
	timer->started_at_ms = now_ms;
	timer->expires_at_ms = now_ms + (uint64_t)duration_ms;
	timer->duration_ms = duration_ms;
	timer->generation++;
	return KN_AX25_TIMER_OK;
}

void
kn_ax25_timer_stop(struct kn_ax25_timer *timer)
{
	if (timer == NULL)
		return;

	timer->running = 0;
}

uint8_t
kn_ax25_timer_is_expired(const struct kn_ax25_timer *timer, uint64_t now_ms)
{
	if (timer == NULL || timer->running == 0)
		return 0;

	return now_ms >= timer->expires_at_ms ? 1 : 0;
}

uint8_t
kn_ax25_timer_is_running(const struct kn_ax25_timer *timer)
{
	if (timer == NULL)
		return 0;

	return timer->running != 0 ? 1 : 0;
}
