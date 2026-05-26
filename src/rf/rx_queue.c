/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rx_queue.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/callsign.h"
#include "kilonode/rx_queue.h"

static uint8_t callsign_equal(const struct kn_callsign *,
	const struct kn_callsign *);
static const struct kn_rx_event *event_at(const struct kn_rx_queue *,
	size_t);
static enum kn_rx_queue_error list_match(const struct kn_rx_queue *,
	const struct kn_rx_event **, size_t, size_t *, const char *,
	const struct kn_callsign *, const struct kn_callsign *);

void
kn_rx_queue_clear(struct kn_rx_queue *queue)
{
	size_t max_events;
	size_t preview_bytes;
	uint8_t enabled;

	if (queue == NULL)
		return;

	max_events = queue->max_events;
	preview_bytes = queue->preview_bytes;
	enabled = queue->enabled;
	memset(queue, 0, sizeof(*queue));
	queue->max_events = max_events;
	queue->preview_bytes = preview_bytes;
	queue->enabled = enabled;
	queue->next_id = 1;
}

size_t
kn_rx_queue_count(const struct kn_rx_queue *queue)
{
	if (queue == NULL)
		return 0;

	return queue->count;
}

const struct kn_rx_event *
kn_rx_queue_get(const struct kn_rx_queue *queue, uint64_t id)
{
	size_t i;
	const struct kn_rx_event *event;

	if (queue == NULL || id == 0)
		return NULL;

	for (i = 0; i < queue->count; i++) {
		event = event_at(queue, i);
		if (event != NULL && event->id == id)
			return event;
	}

	return NULL;
}

void
kn_rx_queue_init(struct kn_rx_queue *queue, size_t max_events,
	size_t preview_bytes, uint8_t enabled)
{
	if (queue == NULL)
		return;

	memset(queue, 0, sizeof(*queue));
	if (max_events == 0 || max_events > KN_RX_QUEUE_MAX)
		queue->max_events = KN_RX_QUEUE_DEFAULT_MAX;
	else
		queue->max_events = max_events;
	if (preview_bytes == 0 || preview_bytes > KN_RX_EVENT_PREVIEW_MAX)
		queue->preview_bytes = KN_RX_EVENT_PREVIEW_DEFAULT;
	else
		queue->preview_bytes = preview_bytes;
	queue->enabled = enabled;
	queue->next_id = 1;
}

enum kn_rx_queue_error
kn_rx_queue_list(const struct kn_rx_queue *queue,
	const struct kn_rx_event **events, size_t max_events, size_t *count)
{
	return list_match(queue, events, max_events, count, NULL, NULL, NULL);
}

enum kn_rx_queue_error
kn_rx_queue_list_by_destination(const struct kn_rx_queue *queue,
	const struct kn_callsign *destination, const struct kn_rx_event **events,
	size_t max_events, size_t *count)
{
	if (destination == NULL)
		return KN_RX_QUEUE_ERR_INVALID_ARGUMENT;

	return list_match(queue, events, max_events, count, NULL, NULL,
	    destination);
}

enum kn_rx_queue_error
kn_rx_queue_list_by_port(const struct kn_rx_queue *queue, const char *port,
	const struct kn_rx_event **events, size_t max_events, size_t *count)
{
	if (port == NULL || port[0] == '\0')
		return KN_RX_QUEUE_ERR_INVALID_ARGUMENT;

	return list_match(queue, events, max_events, count, port, NULL, NULL);
}

enum kn_rx_queue_error
kn_rx_queue_list_by_source(const struct kn_rx_queue *queue,
	const struct kn_callsign *source, const struct kn_rx_event **events,
	size_t max_events, size_t *count)
{
	if (source == NULL)
		return KN_RX_QUEUE_ERR_INVALID_ARGUMENT;

	return list_match(queue, events, max_events, count, NULL, source, NULL);
}

const struct kn_rx_event *
kn_rx_queue_newest(const struct kn_rx_queue *queue)
{
	if (queue == NULL || queue->count == 0)
		return NULL;

	return event_at(queue, queue->count - 1);
}

enum kn_rx_queue_error
kn_rx_queue_push(struct kn_rx_queue *queue, const struct kn_rx_event *event)
{
	size_t index;

	if (queue == NULL || event == NULL || queue->max_events == 0)
		return KN_RX_QUEUE_ERR_INVALID_ARGUMENT;
	if (queue->enabled == 0)
		return KN_RX_QUEUE_OK;

	if (queue->count < queue->max_events) {
		index = (queue->start + queue->count) % queue->max_events;
		queue->count++;
	} else {
		index = queue->start;
		queue->start = (queue->start + 1) % queue->max_events;
	}

	queue->events[index] = *event;
	return KN_RX_QUEUE_OK;
}

uint64_t
kn_rx_queue_reserve_id(struct kn_rx_queue *queue)
{
	uint64_t id;

	if (queue == NULL)
		return 0;

	id = queue->next_id;
	if (queue->next_id < UINT64_MAX)
		queue->next_id++;
	else
		queue->next_id = 1;

	return id;
}

static uint8_t
callsign_equal(const struct kn_callsign *left, const struct kn_callsign *right)
{
	return (uint8_t)(strcmp(left->call, right->call) == 0 &&
	    left->ssid == right->ssid);
}

static const struct kn_rx_event *
event_at(const struct kn_rx_queue *queue, size_t offset)
{
	if (queue == NULL || offset >= queue->count || queue->max_events == 0)
		return NULL;

	return &queue->events[(queue->start + offset) % queue->max_events];
}

static enum kn_rx_queue_error
list_match(const struct kn_rx_queue *queue, const struct kn_rx_event **events,
	size_t max_events, size_t *count, const char *port,
	const struct kn_callsign *source, const struct kn_callsign *destination)
{
	const struct kn_rx_event *event;
	size_t i;
	size_t out_count;

	if (queue == NULL || events == NULL || count == NULL)
		return KN_RX_QUEUE_ERR_INVALID_ARGUMENT;

	out_count = 0;
	for (i = queue->count; i > 0; i--) {
		event = event_at(queue, i - 1);
		if (event == NULL)
			continue;
		if (port != NULL && strcmp(event->port_name, port) != 0)
			continue;
		if (source != NULL &&
		    callsign_equal(&event->source, source) == 0)
			continue;
		if (destination != NULL &&
		    callsign_equal(&event->destination, destination) == 0)
			continue;
		if (out_count < max_events)
			events[out_count] = event;
		out_count++;
	}

	*count = out_count < max_events ? out_count : max_events;
	return KN_RX_QUEUE_OK;
}
