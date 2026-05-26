/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/core/heard.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/heard.h"

static size_t entry_find(const struct kn_heard_table *, const char *,
	const struct kn_callsign *);
static uint8_t entry_matches(const struct kn_heard_entry *, const char *,
	const struct kn_callsign *);
static size_t evict_index(const struct kn_heard_table *);
static enum kn_heard_error port_copy(char *, size_t, const char *);
static uint8_t source_valid(const struct kn_callsign *);

void
kn_heard_clear(struct kn_heard_table *table)
{
	size_t max_entries;

	if (table == NULL)
		return;

	max_entries = table->max_entries;
	memset(table, 0, sizeof(*table));
	table->max_entries = max_entries;
}

size_t
kn_heard_count(const struct kn_heard_table *table)
{
	if (table == NULL)
		return 0;

	return table->count;
}

const struct kn_heard_entry *
kn_heard_entries(const struct kn_heard_table *table)
{
	if (table == NULL)
		return NULL;

	return table->entries;
}

int
kn_heard_format_callsign(const struct kn_callsign *callsign, char *buf,
	size_t bufsiz)
{
	return kn_callsign_format(callsign, buf, bufsiz);
}

void
kn_heard_init(struct kn_heard_table *table, size_t max_entries)
{
	if (table == NULL)
		return;

	memset(table, 0, sizeof(*table));
	if (max_entries == 0 || max_entries > KN_HEARD_DEFAULT_MAX)
		table->max_entries = KN_HEARD_DEFAULT_MAX;
	else
		table->max_entries = max_entries;
}

enum kn_heard_error
kn_heard_update(struct kn_heard_table *table, const char *port_name,
	const struct kn_ax25_frame *frame, uint64_t now)
{
	struct kn_heard_entry *entry;
	size_t index;

	if (table == NULL || port_name == NULL || frame == NULL ||
	    table->max_entries == 0 || source_valid(&frame->source.callsign) == 0)
		return KN_HEARD_ERR_INVALID_ARGUMENT;

	index = entry_find(table, port_name, &frame->source.callsign);
	if (index == table->count) {
		if (table->count < table->max_entries) {
			entry = &table->entries[table->count++];
			memset(entry, 0, sizeof(*entry));
		} else {
			entry = &table->entries[evict_index(table)];
			memset(entry, 0, sizeof(*entry));
		}

		if (port_copy(entry->port_name, sizeof(entry->port_name),
		    port_name) != KN_HEARD_OK) {
			if (table->count > 0 &&
			    entry == &table->entries[table->count - 1])
				table->count--;
			return KN_HEARD_ERR_INVALID_ARGUMENT;
		}
		entry->source = frame->source.callsign;
		entry->first_heard = now;
	} else {
		entry = &table->entries[index];
	}

	entry->last_heard = now;
	entry->frame_count++;
	entry->last_destination = frame->destination.callsign;
	entry->digipeater_count = frame->digipeater_count;
	if (entry->digipeater_count > KN_HEARD_MAX_DIGIS)
		entry->digipeater_count = KN_HEARD_MAX_DIGIS;
	if (entry->digipeater_count > 0) {
		memcpy(entry->digipeaters, frame->digipeaters,
		    entry->digipeater_count * sizeof(entry->digipeaters[0]));
	}
	entry->last_control = frame->control;
	entry->last_pid = frame->pid;
	entry->has_pid = frame->has_pid;
	entry->last_payload_len = frame->payload_len;
	entry->last_ui = (uint8_t)((frame->control & 0xefU) ==
	    KN_AX25_CONTROL_UI);

	return KN_HEARD_OK;
}

static size_t
entry_find(const struct kn_heard_table *table, const char *port_name,
	const struct kn_callsign *callsign)
{
	size_t i;

	for (i = 0; i < table->count; i++) {
		if (entry_matches(&table->entries[i], port_name, callsign) != 0)
			return i;
	}

	return table->count;
}

static uint8_t
entry_matches(const struct kn_heard_entry *entry, const char *port_name,
	const struct kn_callsign *callsign)
{
	if (strcmp(entry->port_name, port_name) != 0)
		return 0;
	if (strcmp(entry->source.call, callsign->call) != 0)
		return 0;
	if (entry->source.ssid != callsign->ssid)
		return 0;

	return 1;
}

static size_t
evict_index(const struct kn_heard_table *table)
{
	size_t i;
	size_t oldest;

	oldest = 0;
	for (i = 1; i < table->count; i++) {
		if (table->entries[i].last_heard <
		    table->entries[oldest].last_heard)
			oldest = i;
	}

	return oldest;
}

static enum kn_heard_error
port_copy(char *dst, size_t dst_len, const char *src)
{
	size_t len;

	len = strlen(src);
	if (len == 0 || len >= dst_len)
		return KN_HEARD_ERR_INVALID_ARGUMENT;

	memcpy(dst, src, len + 1);
	return KN_HEARD_OK;
}

static uint8_t
source_valid(const struct kn_callsign *callsign)
{
	char buf[KN_CALLSIGN_MAX + 4];

	if (callsign == NULL)
		return 0;

	return kn_callsign_format(callsign, buf, sizeof(buf)) == 0 ? 1 : 0;
}
