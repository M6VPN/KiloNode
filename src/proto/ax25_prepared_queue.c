/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_queue.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_queue.h"

static int port_matches(const struct kn_ax25_prepared_frame *, const char *);

static int
port_matches(const struct kn_ax25_prepared_frame *frame, const char *port)
{
	if (frame == NULL || port == NULL)
		return 0;

	return strcmp(frame->port_name, port) == 0 ? 1 : 0;
}

size_t
kn_ax25_prepared_queue_count(const struct kn_ax25_prepared_queue *queue)
{
	if (queue == NULL)
		return 0;

	return queue->count;
}

void
kn_ax25_prepared_queue_free(struct kn_ax25_prepared_queue *queue)
{
	if (queue == NULL)
		return;

	kn_ax25_prepared_queue_reset(queue);
}

const struct kn_ax25_prepared_frame *
kn_ax25_prepared_queue_get(const struct kn_ax25_prepared_queue *queue,
	uint64_t id)
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

void
kn_ax25_prepared_queue_init(struct kn_ax25_prepared_queue *queue)
{
	if (queue == NULL)
		return;

	memset(queue, 0, sizeof(*queue));
	queue->max_frames = KN_AX25_PREPARED_QUEUE_DEFAULT_MAX;
	queue->next_id = 1;
}

enum kn_ax25_prepared_queue_error
kn_ax25_prepared_queue_list(const struct kn_ax25_prepared_queue *queue,
	const struct kn_ax25_prepared_frame **frames, size_t frames_len,
	size_t *count)
{
	size_t i;

	if (queue == NULL || frames == NULL || count == NULL)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_ARGUMENT;

	*count = 0;
	for (i = 0; i < queue->count; i++) {
		if (*count >= frames_len)
			return KN_AX25_PREPARED_QUEUE_ERR_BUFFER;
		frames[*count] = &queue->frames[i];
		(*count)++;
	}

	return KN_AX25_PREPARED_QUEUE_OK;
}

enum kn_ax25_prepared_queue_error
kn_ax25_prepared_queue_list_by_connection(
	const struct kn_ax25_prepared_queue *queue, uint32_t connection_id,
	const struct kn_ax25_prepared_frame **frames, size_t frames_len,
	size_t *count)
{
	size_t i;

	if (queue == NULL || frames == NULL || count == NULL ||
	    connection_id == 0)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_ARGUMENT;

	*count = 0;
	for (i = 0; i < queue->count; i++) {
		if (queue->frames[i].connection_id != connection_id)
			continue;
		if (*count >= frames_len)
			return KN_AX25_PREPARED_QUEUE_ERR_BUFFER;
		frames[*count] = &queue->frames[i];
		(*count)++;
	}

	return KN_AX25_PREPARED_QUEUE_OK;
}

enum kn_ax25_prepared_queue_error
kn_ax25_prepared_queue_list_by_port(
	const struct kn_ax25_prepared_queue *queue, const char *port,
	const struct kn_ax25_prepared_frame **frames, size_t frames_len,
	size_t *count)
{
	size_t i;

	if (queue == NULL || port == NULL || frames == NULL || count == NULL)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_ARGUMENT;

	*count = 0;
	for (i = 0; i < queue->count; i++) {
		if (port_matches(&queue->frames[i], port) == 0)
			continue;
		if (*count >= frames_len)
			return KN_AX25_PREPARED_QUEUE_ERR_BUFFER;
		frames[*count] = &queue->frames[i];
		(*count)++;
	}

	return KN_AX25_PREPARED_QUEUE_OK;
}

enum kn_ax25_prepared_queue_error
kn_ax25_prepared_queue_push(struct kn_ax25_prepared_queue *queue,
	const struct kn_ax25_prepared_frame *frame, uint64_t *id)
{
	struct kn_ax25_prepared_frame copy;

	if (queue == NULL || frame == NULL || id == NULL)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_ARGUMENT;
	if (queue->max_frames == 0 ||
	    queue->max_frames > KN_AX25_PREPARED_QUEUE_MAX)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_VALUE;
	if (queue->count >= queue->max_frames)
		return KN_AX25_PREPARED_QUEUE_ERR_FULL;

	copy = *frame;
	copy.id = queue->next_id;
	queue->frames[queue->count] = copy;
	*id = copy.id;
	queue->count++;
	queue->next_id++;
	return KN_AX25_PREPARED_QUEUE_OK;
}

void
kn_ax25_prepared_queue_reset(struct kn_ax25_prepared_queue *queue)
{
	size_t max_frames;

	if (queue == NULL)
		return;

	max_frames = queue->max_frames;
	memset(queue, 0, sizeof(*queue));
	if (max_frames == 0)
		max_frames = KN_AX25_PREPARED_QUEUE_DEFAULT_MAX;
	queue->max_frames = max_frames;
	queue->next_id = 1;
}

enum kn_ax25_prepared_queue_error
kn_ax25_prepared_queue_set_max(struct kn_ax25_prepared_queue *queue,
	size_t max_frames)
{
	if (queue == NULL)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_ARGUMENT;
	if (max_frames == 0 || max_frames > KN_AX25_PREPARED_QUEUE_MAX)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_VALUE;
	if (queue->count > max_frames)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_VALUE;

	queue->max_frames = max_frames;
	return KN_AX25_PREPARED_QUEUE_OK;
}

enum kn_ax25_prepared_queue_error
kn_ax25_prepared_queue_format(const struct kn_ax25_prepared_queue *queue,
	char *buf, size_t bufsiz)
{
	int needed;

	if (queue == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_QUEUE_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz, "count=%llu max=%llu next_id=%llu",
	    (unsigned long long)queue->count,
	    (unsigned long long)queue->max_frames,
	    (unsigned long long)queue->next_id);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_QUEUE_ERR_BUFFER;

	return KN_AX25_PREPARED_QUEUE_OK;
}
