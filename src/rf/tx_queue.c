/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/tx_queue.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/tx_queue.h"

static enum kn_tx_queue_error list_match(const struct kn_tx_queue *,
	const char *, const struct kn_tx_frame **, size_t, size_t *);

void
kn_tx_queue_clear(struct kn_tx_queue *queue)
{
	if (queue == NULL)
		return;

	memset(queue->frames, 0, sizeof(queue->frames));
	queue->count = 0;
	queue->next_id = 1;
}

size_t
kn_tx_queue_count(const struct kn_tx_queue *queue)
{
	if (queue == NULL)
		return 0;

	return queue->count;
}

enum kn_tx_queue_error
kn_tx_queue_enqueue(struct kn_tx_queue *queue, const struct kn_tx_frame *frame)
{
	enum kn_tx_policy_error policy_rc;

	if (queue == NULL || frame == NULL)
		return KN_TX_QUEUE_ERR_INVALID_ARGUMENT;
	if (queue->count >= queue->max_frames)
		return KN_TX_QUEUE_ERR_FULL;
	policy_rc = kn_tx_policy_allow_enqueue(&queue->policy,
	    frame->payload_len);
	if (policy_rc != KN_TX_POLICY_OK)
		return KN_TX_QUEUE_ERR_POLICY;

	queue->frames[queue->count++] = *frame;
	return KN_TX_QUEUE_OK;
}

const struct kn_tx_frame *
kn_tx_queue_get(const struct kn_tx_queue *queue, uint64_t id)
{
	size_t i;

	if (queue == NULL || id == 0)
		return NULL;

	for (i = 0; i < queue->count; i++) {
		if (queue->frames[i].id == id)
			return &queue->frames[i];
	}

	return NULL;
}

enum kn_tx_queue_error
kn_tx_queue_init(struct kn_tx_queue *queue, const struct kn_tx_policy *policy)
{
	if (queue == NULL || policy == NULL)
		return KN_TX_QUEUE_ERR_INVALID_ARGUMENT;
	if (kn_tx_policy_validate(policy) != KN_TX_POLICY_OK)
		return KN_TX_QUEUE_ERR_POLICY;

	memset(queue, 0, sizeof(*queue));
	queue->max_frames = policy->max_queued;
	queue->next_id = 1;
	queue->policy = *policy;
	return KN_TX_QUEUE_OK;
}

enum kn_tx_queue_error
kn_tx_queue_list(const struct kn_tx_queue *queue,
	const struct kn_tx_frame **frames, size_t max_frames, size_t *count)
{
	return list_match(queue, NULL, frames, max_frames, count);
}

enum kn_tx_queue_error
kn_tx_queue_list_by_port(const struct kn_tx_queue *queue, const char *port,
	const struct kn_tx_frame **frames, size_t max_frames, size_t *count)
{
	if (port == NULL || port[0] == '\0')
		return KN_TX_QUEUE_ERR_INVALID_ARGUMENT;

	return list_match(queue, port, frames, max_frames, count);
}

enum kn_tx_queue_error
kn_tx_queue_mark_dropped(struct kn_tx_queue *queue, uint64_t id)
{
	struct kn_tx_frame *frame;

	frame = kn_tx_queue_mutable_get(queue, id);
	if (frame == NULL)
		return KN_TX_QUEUE_ERR_NOT_FOUND;

	frame->status = KN_TX_FRAME_DROPPED;
	return KN_TX_QUEUE_OK;
}

enum kn_tx_queue_error
kn_tx_queue_mark_failed(struct kn_tx_queue *queue, uint64_t id, int error)
{
	struct kn_tx_frame *frame;

	frame = kn_tx_queue_mutable_get(queue, id);
	if (frame == NULL)
		return KN_TX_QUEUE_ERR_NOT_FOUND;

	frame->status = KN_TX_FRAME_FAILED;
	frame->last_error = error;
	return KN_TX_QUEUE_OK;
}

enum kn_tx_queue_error
kn_tx_queue_mark_sent(struct kn_tx_queue *queue, uint64_t id)
{
	struct kn_tx_frame *frame;

	frame = kn_tx_queue_mutable_get(queue, id);
	if (frame == NULL)
		return KN_TX_QUEUE_ERR_NOT_FOUND;

	frame->status = KN_TX_FRAME_SENT;
	return KN_TX_QUEUE_OK;
}

struct kn_tx_frame *
kn_tx_queue_mutable_get(struct kn_tx_queue *queue, uint64_t id)
{
	size_t i;

	if (queue == NULL || id == 0)
		return NULL;

	for (i = 0; i < queue->count; i++) {
		if (queue->frames[i].id == id)
			return &queue->frames[i];
	}

	return NULL;
}

struct kn_tx_frame *
kn_tx_queue_pop_next_for_port(struct kn_tx_queue *queue, const char *port)
{
	size_t i;

	if (queue == NULL || port == NULL || port[0] == '\0')
		return NULL;

	for (i = 0; i < queue->count; i++) {
		if (strcmp(queue->frames[i].port_name, port) == 0 &&
		    (queue->frames[i].status == KN_TX_FRAME_QUEUED ||
		    queue->frames[i].status == KN_TX_FRAME_DRY_RUN))
			return &queue->frames[i];
	}

	return NULL;
}

uint64_t
kn_tx_queue_reserve_id(struct kn_tx_queue *queue)
{
	uint64_t id;

	if (queue == NULL)
		return 0;

	id = queue->next_id++;
	if (queue->next_id == 0)
		queue->next_id = 1;

	return id;
}

static enum kn_tx_queue_error
list_match(const struct kn_tx_queue *queue, const char *port,
	const struct kn_tx_frame **frames, size_t max_frames, size_t *count)
{
	size_t i;
	size_t out;

	if (queue == NULL || frames == NULL || count == NULL)
		return KN_TX_QUEUE_ERR_INVALID_ARGUMENT;

	out = 0;
	for (i = 0; i < queue->count && out < max_frames; i++) {
		if (port != NULL &&
		    strcmp(queue->frames[i].port_name, port) != 0)
			continue;
		frames[out++] = &queue->frames[i];
	}

	*count = out;
	return KN_TX_QUEUE_OK;
}
