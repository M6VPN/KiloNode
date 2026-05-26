/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rx_session.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/callsign.h"
#include "kilonode/rx_session.h"

static void make_event(struct kn_rx_event *, uint64_t, uint64_t,
	const char *, const char *, const char *, enum kn_rx_frame_kind);
static int test_create_update(void);
static int test_eviction(void);
static int test_filters(void);

int
main(void)
{
	if (test_create_update() != 0)
		return 1;
	if (test_filters() != 0)
		return 1;
	if (test_eviction() != 0)
		return 1;

	return 0;
}

static void
make_event(struct kn_rx_event *event, uint64_t id, uint64_t now,
	const char *port, const char *source, const char *destination,
	enum kn_rx_frame_kind kind)
{
	kn_rx_event_clear(event);
	event->id = id;
	event->timestamp = now;
	(void)snprintf(event->port_name, sizeof(event->port_name), "%s", port);
	(void)kn_callsign_parse(source, &event->source);
	(void)kn_callsign_parse(destination, &event->destination);
	event->kind = kind;
	event->control = kind == KN_RX_FRAME_I ? 0x00 : 0x03;
	event->pid = 0xf0;
	event->has_pid = 1;
}

static int
test_create_update(void)
{
	struct kn_rx_session_table table;
	struct kn_rx_event event;
	const struct kn_rx_session_entry *entry;

	kn_rx_session_init(&table, 4);
	make_event(&event, 1, 10, "kiss0", "M6VPN-1", "CQ",
	    KN_RX_FRAME_UI);
	if (kn_rx_session_update(&table, &event) != KN_RX_SESSION_OK)
		return 1;
	make_event(&event, 2, 20, "kiss0", "M6VPN-1", "CQ",
	    KN_RX_FRAME_I);
	if (kn_rx_session_update(&table, &event) != KN_RX_SESSION_OK)
		return 1;
	if (kn_rx_session_count(&table) != 1)
		return 1;
	entry = kn_rx_session_entries(&table);
	if (entry->first_seen != 10 || entry->last_seen != 20)
		return 1;
	if (entry->frame_count != 2 || entry->ui_count != 1 ||
	    entry->i_count != 1)
		return 1;

	return entry->last_event_id == 2 ? 0 : 1;
}

static int
test_eviction(void)
{
	struct kn_rx_session_table table;
	struct kn_rx_event event;

	kn_rx_session_init(&table, 2);
	make_event(&event, 1, 10, "kiss0", "A", "CQ", KN_RX_FRAME_UI);
	(void)kn_rx_session_update(&table, &event);
	make_event(&event, 2, 20, "kiss0", "B", "CQ", KN_RX_FRAME_UI);
	(void)kn_rx_session_update(&table, &event);
	make_event(&event, 3, 30, "kiss0", "C", "CQ", KN_RX_FRAME_UI);
	(void)kn_rx_session_update(&table, &event);

	if (kn_rx_session_count(&table) != 2)
		return 1;

	return kn_rx_session_entries(&table)[0].last_event_id == 3 ? 0 : 1;
}

static int
test_filters(void)
{
	struct kn_rx_session_table table;
	struct kn_rx_event event;
	struct kn_callsign source;
	const struct kn_rx_session_entry *entries[4];
	size_t count;

	kn_rx_session_init(&table, 4);
	make_event(&event, 1, 10, "kiss0", "M6VPN-1", "CQ",
	    KN_RX_FRAME_UI);
	(void)kn_rx_session_update(&table, &event);
	make_event(&event, 2, 20, "kiss1", "N0CALL", "CQ",
	    KN_RX_FRAME_U);
	(void)kn_rx_session_update(&table, &event);

	if (kn_rx_session_list_by_port(&table, "kiss0", entries, 4,
	    &count) != KN_RX_SESSION_OK || count != 1 ||
	    entries[0]->last_event_id != 1)
		return 1;
	(void)kn_callsign_parse("N0CALL", &source);
	if (kn_rx_session_list_by_source(&table, &source, entries, 4,
	    &count) != KN_RX_SESSION_OK || count != 1 ||
	    entries[0]->last_event_id != 2)
		return 1;

	return 0;
}
