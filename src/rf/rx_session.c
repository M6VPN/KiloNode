/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rx_session.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/callsign.h"
#include "kilonode/rx_session.h"

static uint8_t callsign_equal(const struct kn_callsign *,
	const struct kn_callsign *);
static size_t evict_index(const struct kn_rx_session_table *);
static size_t find_entry(const struct kn_rx_session_table *,
	const struct kn_rx_event *);
static enum kn_rx_session_error list_match(
	const struct kn_rx_session_table *, const struct kn_rx_session_entry **,
	size_t, size_t *, const char *, const struct kn_callsign *);

void
kn_rx_session_clear(struct kn_rx_session_table *table)
{
	size_t max_sessions;

	if (table == NULL)
		return;

	max_sessions = table->max_sessions;
	memset(table, 0, sizeof(*table));
	table->max_sessions = max_sessions;
}

size_t
kn_rx_session_count(const struct kn_rx_session_table *table)
{
	if (table == NULL)
		return 0;

	return table->count;
}

const struct kn_rx_session_entry *
kn_rx_session_entries(const struct kn_rx_session_table *table)
{
	if (table == NULL)
		return NULL;

	return table->entries;
}

void
kn_rx_session_init(struct kn_rx_session_table *table, size_t max_sessions)
{
	if (table == NULL)
		return;

	memset(table, 0, sizeof(*table));
	if (max_sessions == 0 || max_sessions > KN_RX_SESSION_MAX)
		table->max_sessions = KN_RX_SESSION_DEFAULT_MAX;
	else
		table->max_sessions = max_sessions;
}

enum kn_rx_session_error
kn_rx_session_list_by_port(const struct kn_rx_session_table *table,
	const char *port, const struct kn_rx_session_entry **entries,
	size_t max_entries, size_t *count)
{
	if (port == NULL || port[0] == '\0')
		return KN_RX_SESSION_ERR_INVALID_ARGUMENT;

	return list_match(table, entries, max_entries, count, port, NULL);
}

enum kn_rx_session_error
kn_rx_session_list_by_source(const struct kn_rx_session_table *table,
	const struct kn_callsign *source,
	const struct kn_rx_session_entry **entries, size_t max_entries,
	size_t *count)
{
	if (source == NULL)
		return KN_RX_SESSION_ERR_INVALID_ARGUMENT;

	return list_match(table, entries, max_entries, count, NULL, source);
}

enum kn_rx_session_error
kn_rx_session_update(struct kn_rx_session_table *table,
	const struct kn_rx_event *event)
{
	struct kn_rx_session_entry *entry;
	size_t index;

	if (table == NULL || event == NULL || table->max_sessions == 0)
		return KN_RX_SESSION_ERR_INVALID_ARGUMENT;
	if (event->port_name[0] == '\0')
		return KN_RX_SESSION_ERR_INVALID_ARGUMENT;

	index = find_entry(table, event);
	if (index == table->count) {
		if (table->count < table->max_sessions) {
			entry = &table->entries[table->count++];
			memset(entry, 0, sizeof(*entry));
		} else {
			entry = &table->entries[evict_index(table)];
			memset(entry, 0, sizeof(*entry));
		}
		entry->source = event->source;
		entry->destination = event->destination;
		(void)snprintf(entry->port_name, sizeof(entry->port_name),
		    "%s", event->port_name);
		entry->first_seen = event->timestamp;
	} else {
		entry = &table->entries[index];
	}

	entry->last_seen = event->timestamp;
	entry->frame_count++;
	entry->last_control = event->control;
	entry->last_pid = event->pid;
	entry->has_pid = event->has_pid;
	entry->last_event_id = event->id;

	switch (event->kind) {
	case KN_RX_FRAME_UI:
		entry->ui_count++;
		break;
	case KN_RX_FRAME_I:
		entry->i_count++;
		break;
	case KN_RX_FRAME_S:
		entry->s_count++;
		break;
	case KN_RX_FRAME_U:
		entry->u_count++;
		break;
	case KN_RX_FRAME_MALFORMED:
		entry->malformed_count++;
		break;
	case KN_RX_FRAME_UNKNOWN:
		break;
	}

	return KN_RX_SESSION_OK;
}

static uint8_t
callsign_equal(const struct kn_callsign *left, const struct kn_callsign *right)
{
	return (uint8_t)(strcmp(left->call, right->call) == 0 &&
	    left->ssid == right->ssid);
}

static size_t
evict_index(const struct kn_rx_session_table *table)
{
	size_t i;
	size_t oldest;

	oldest = 0;
	for (i = 1; i < table->count; i++) {
		if (table->entries[i].last_seen <
		    table->entries[oldest].last_seen)
			oldest = i;
	}

	return oldest;
}

static size_t
find_entry(const struct kn_rx_session_table *table,
	const struct kn_rx_event *event)
{
	size_t i;

	for (i = 0; i < table->count; i++) {
		if (strcmp(table->entries[i].port_name,
		    event->port_name) == 0 &&
		    callsign_equal(&table->entries[i].source,
		    &event->source) != 0 &&
		    callsign_equal(&table->entries[i].destination,
		    &event->destination) != 0)
			return i;
	}

	return table->count;
}

static enum kn_rx_session_error
list_match(const struct kn_rx_session_table *table,
	const struct kn_rx_session_entry **entries, size_t max_entries,
	size_t *count, const char *port, const struct kn_callsign *source)
{
	size_t i;
	size_t out_count;

	if (table == NULL || entries == NULL || count == NULL)
		return KN_RX_SESSION_ERR_INVALID_ARGUMENT;

	out_count = 0;
	for (i = 0; i < table->count; i++) {
		if (port != NULL &&
		    strcmp(table->entries[i].port_name, port) != 0)
			continue;
		if (source != NULL &&
		    callsign_equal(&table->entries[i].source, source) == 0)
			continue;
		if (out_count < max_entries)
			entries[out_count] = &table->entries[i];
		out_count++;
	}

	*count = out_count < max_entries ? out_count : max_entries;
	return KN_RX_SESSION_OK;
}
