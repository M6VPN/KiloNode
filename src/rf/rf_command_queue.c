/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rf_command_queue.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/callsign.h"
#include "kilonode/rf_command_queue.h"

enum queue_filter {
	FILTER_ALL = 0,
	FILTER_PORT,
	FILTER_SOURCE
};

static size_t event_index(const struct kn_rf_command_queue *, size_t);
static enum kn_rf_command_queue_error list_match(
	const struct kn_rf_command_queue *, enum queue_filter, const char *,
	const struct kn_rf_command_event **, size_t, size_t *);

void
kn_rf_command_queue_clear(struct kn_rf_command_queue *queue)
{
	if (queue == NULL)
		return;

	memset(queue->events, 0, sizeof(queue->events));
	queue->count = 0;
	queue->head = 0;
	queue->next_id = 1;
}

size_t
kn_rf_command_queue_count(const struct kn_rf_command_queue *queue)
{
	if (queue == NULL)
		return 0;

	return queue->count;
}

const struct kn_rf_command_event *
kn_rf_command_queue_get(const struct kn_rf_command_queue *queue, uint64_t id)
{
	size_t i;
	size_t idx;

	if (queue == NULL || id == 0)
		return NULL;

	for (i = 0; i < queue->count; i++) {
		idx = event_index(queue, i);
		if (queue->events[idx].id == id)
			return &queue->events[idx];
	}

	return NULL;
}

enum kn_rf_command_queue_error
kn_rf_command_queue_init(struct kn_rf_command_queue *queue, size_t max_events)
{
	if (queue == NULL ||
	    max_events < KN_CONFIG_RF_COMMAND_EVENTS_MIN ||
	    max_events > KN_CONFIG_RF_COMMAND_EVENTS_MAX)
		return KN_RF_COMMAND_QUEUE_ERR_INVALID_ARGUMENT;

	memset(queue, 0, sizeof(*queue));
	queue->max_events = max_events;
	queue->next_id = 1;
	return KN_RF_COMMAND_QUEUE_OK;
}

enum kn_rf_command_queue_error
kn_rf_command_queue_list(const struct kn_rf_command_queue *queue,
	const struct kn_rf_command_event **events, size_t max_events,
	size_t *count)
{
	return list_match(queue, FILTER_ALL, NULL, events, max_events, count);
}

enum kn_rf_command_queue_error
kn_rf_command_queue_list_by_port(const struct kn_rf_command_queue *queue,
	const char *port, const struct kn_rf_command_event **events,
	size_t max_events, size_t *count)
{
	if (port == NULL || port[0] == '\0')
		return KN_RF_COMMAND_QUEUE_ERR_INVALID_ARGUMENT;

	return list_match(queue, FILTER_PORT, port, events, max_events, count);
}

enum kn_rf_command_queue_error
kn_rf_command_queue_list_by_source(const struct kn_rf_command_queue *queue,
	const char *source, const struct kn_rf_command_event **events,
	size_t max_events, size_t *count)
{
	struct kn_callsign parsed;

	if (source == NULL || kn_callsign_parse(source, &parsed) != 0)
		return KN_RF_COMMAND_QUEUE_ERR_INVALID_ARGUMENT;

	return list_match(queue, FILTER_SOURCE, source, events, max_events,
	    count);
}

enum kn_rf_command_queue_error
kn_rf_command_queue_push(struct kn_rf_command_queue *queue,
	const struct kn_rf_command_event *event)
{
	size_t idx;

	if (queue == NULL || event == NULL || queue->max_events == 0)
		return KN_RF_COMMAND_QUEUE_ERR_INVALID_ARGUMENT;

	if (queue->count < queue->max_events) {
		idx = (queue->head + queue->count) % queue->max_events;
		queue->count++;
	} else {
		idx = queue->head;
		queue->head = (queue->head + 1) % queue->max_events;
	}

	queue->events[idx] = *event;
	return KN_RF_COMMAND_QUEUE_OK;
}

uint64_t
kn_rf_command_queue_reserve_id(struct kn_rf_command_queue *queue)
{
	uint64_t id;

	if (queue == NULL)
		return 0;

	id = queue->next_id++;
	if (queue->next_id == 0)
		queue->next_id = 1;
	return id;
}

static size_t
event_index(const struct kn_rf_command_queue *queue, size_t offset)
{
	return (queue->head + offset) % queue->max_events;
}

static enum kn_rf_command_queue_error
list_match(const struct kn_rf_command_queue *queue, enum queue_filter filter,
	const char *value, const struct kn_rf_command_event **events,
	size_t max_events, size_t *count)
{
	char source[KN_CALLSIGN_MAX + 4];
	size_t i;
	size_t idx;
	size_t out;

	if (queue == NULL || events == NULL || count == NULL)
		return KN_RF_COMMAND_QUEUE_ERR_INVALID_ARGUMENT;

	out = 0;
	for (i = queue->count; i > 0 && out < max_events; i--) {
		idx = event_index(queue, i - 1);
		if (filter == FILTER_PORT &&
		    strcmp(queue->events[idx].port_name, value) != 0)
			continue;
		if (filter == FILTER_SOURCE) {
			if (kn_callsign_format(&queue->events[idx].source,
			    source, sizeof(source)) != 0)
				continue;
			if (strcmp(source, value) != 0)
				continue;
		}
		events[out++] = &queue->events[idx];
	}

	*count = out;
	return KN_RF_COMMAND_QUEUE_OK;
}
