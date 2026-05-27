/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_timer_queue.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_timer_queue.h"

static uint8_t timer_less(const struct kn_ax25_timer *,
	const struct kn_ax25_timer *);

static uint8_t
timer_less(const struct kn_ax25_timer *a, const struct kn_ax25_timer *b)
{
	if (a->expires_at_ms != b->expires_at_ms)
		return a->expires_at_ms < b->expires_at_ms ? 1 : 0;
	if (a->connection_id != b->connection_id)
		return a->connection_id < b->connection_id ? 1 : 0;

	return a->kind < b->kind ? 1 : 0;
}

size_t
kn_ax25_timer_queue_count_running(const struct kn_ax25_timer_queue *queue)
{
	size_t i;
	size_t count;

	if (queue == NULL)
		return 0;

	count = 0;
	for (i = 0; i < queue->count; i++) {
		if (queue->timers[i].running != 0)
			count++;
	}

	return count;
}

enum kn_ax25_timer_queue_error
kn_ax25_timer_queue_collect_expired(struct kn_ax25_timer_queue *queue,
	uint64_t now_ms, struct kn_ax25_timer_expiry *expired,
	size_t expired_len, size_t *expired_count)
{
	struct kn_ax25_timer *timer;
	size_t best;
	size_t i;
	size_t out;
	uint8_t found;

	if (queue == NULL || expired_count == NULL)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT;
	if (expired == NULL && expired_len > 0)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT;

	out = 0;
	for (;;) {
		found = 0;
		best = 0;
		for (i = 0; i < queue->count; i++) {
			timer = &queue->timers[i];
			if (kn_ax25_timer_is_expired(timer, now_ms) == 0)
				continue;
			if (found == 0 || timer_less(timer,
			    &queue->timers[best]) != 0) {
				best = i;
				found = 1;
			}
		}
		if (found == 0)
			break;
		if (out >= expired_len)
			break;

		timer = &queue->timers[best];
		expired[out].connection_id = timer->connection_id;
		expired[out].kind = timer->kind;
		expired[out].event = kn_ax25_timer_kind_event(timer->kind);
		expired[out].generation = timer->generation;
		expired[out].planned = expired[out].event ==
		    KN_AX25_CONNECTION_EVENT_NONE ? 1 : 0;
		timer->expiry_count++;
		kn_ax25_timer_stop(timer);
		out++;
	}

	*expired_count = out;
	return KN_AX25_TIMER_QUEUE_OK;
}

enum kn_ax25_timer_queue_error
kn_ax25_timer_queue_find(const struct kn_ax25_timer_queue *queue,
	uint32_t connection_id, enum kn_ax25_timer_kind kind, size_t *index)
{
	size_t i;

	if (queue == NULL || index == NULL)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT;
	if (kind == KN_AX25_TIMER_NONE)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_VALUE;

	for (i = 0; i < queue->count; i++) {
		if (queue->timers[i].connection_id == connection_id &&
		    queue->timers[i].kind == kind) {
			*index = i;
			return KN_AX25_TIMER_QUEUE_OK;
		}
	}

	return KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND;
}

enum kn_ax25_timer_queue_error
kn_ax25_timer_queue_format(const struct kn_ax25_timer_queue *queue,
	char *buf, size_t bufsiz)
{
	int needed;

	if (queue == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz, "timers=%llu running=%llu",
	    (unsigned long long)queue->count,
	    (unsigned long long)kn_ax25_timer_queue_count_running(queue));
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_TIMER_QUEUE_ERR_BUFFER;

	return KN_AX25_TIMER_QUEUE_OK;
}

void
kn_ax25_timer_queue_init(struct kn_ax25_timer_queue *queue)
{
	if (queue == NULL)
		return;

	memset(queue, 0, sizeof(*queue));
}

enum kn_ax25_timer_queue_error
kn_ax25_timer_queue_next_expiry(const struct kn_ax25_timer_queue *queue,
	uint64_t *expires_at_ms)
{
	size_t best;
	size_t i;
	uint8_t found;

	if (queue == NULL || expires_at_ms == NULL)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT;

	found = 0;
	best = 0;
	for (i = 0; i < queue->count; i++) {
		if (queue->timers[i].running == 0)
			continue;
		if (found == 0 || timer_less(&queue->timers[i],
		    &queue->timers[best]) != 0) {
			best = i;
			found = 1;
		}
	}
	if (found == 0)
		return KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND;

	*expires_at_ms = queue->timers[best].expires_at_ms;
	return KN_AX25_TIMER_QUEUE_OK;
}

void
kn_ax25_timer_queue_reset(struct kn_ax25_timer_queue *queue)
{
	if (queue == NULL)
		return;

	memset(queue, 0, sizeof(*queue));
}

enum kn_ax25_timer_queue_error
kn_ax25_timer_queue_start(struct kn_ax25_timer_queue *queue,
	uint32_t connection_id, enum kn_ax25_timer_kind kind, uint64_t now_ms,
	uint32_t duration_ms)
{
	size_t index;
	enum kn_ax25_timer_queue_error rc;

	if (queue == NULL)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT;
	if (kind == KN_AX25_TIMER_NONE)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_VALUE;

	rc = kn_ax25_timer_queue_find(queue, connection_id, kind, &index);
	if (rc == KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND) {
		if (queue->count >= KN_AX25_TIMER_QUEUE_MAX)
			return KN_AX25_TIMER_QUEUE_ERR_FULL;
		index = queue->count;
		kn_ax25_timer_init(&queue->timers[index]);
		queue->count++;
	} else if (rc != KN_AX25_TIMER_QUEUE_OK) {
		return rc;
	}
	if (kn_ax25_timer_start(&queue->timers[index], kind, connection_id,
	    now_ms, duration_ms) != KN_AX25_TIMER_OK)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_VALUE;

	return KN_AX25_TIMER_QUEUE_OK;
}

enum kn_ax25_timer_queue_error
kn_ax25_timer_queue_stop(struct kn_ax25_timer_queue *queue,
	uint32_t connection_id, enum kn_ax25_timer_kind kind)
{
	size_t index;
	enum kn_ax25_timer_queue_error rc;

	if (queue == NULL)
		return KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT;

	rc = kn_ax25_timer_queue_find(queue, connection_id, kind, &index);
	if (rc != KN_AX25_TIMER_QUEUE_OK)
		return rc;

	kn_ax25_timer_stop(&queue->timers[index]);
	return KN_AX25_TIMER_QUEUE_OK;
}
