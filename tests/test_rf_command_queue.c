/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rf_command_queue.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/rf_command_queue.h"

static void make_event(struct kn_rf_command_event *, uint64_t, const char *,
	const char *);
static int test_empty(void);
static int test_eviction(void);
static int test_filters(void);
static int test_push_get(void);

int
main(void)
{
	if (test_empty() != 0)
		return 1;
	if (test_push_get() != 0)
		return 1;
	if (test_filters() != 0)
		return 1;
	if (test_eviction() != 0)
		return 1;

	return 0;
}

static void
make_event(struct kn_rf_command_event *event, uint64_t id, const char *port,
	const char *source)
{
	kn_rf_command_event_clear(event);
	event->id = id;
	(void)snprintf(event->port_name, sizeof(event->port_name), "%s", port);
	(void)kn_callsign_parse(source, &event->source);
	(void)kn_callsign_parse("M6VPN-1", &event->destination);
	event->command = KN_RF_COMMAND_PING;
	event->status = KN_RF_COMMAND_STATUS_OK;
}

static int
test_empty(void)
{
	struct kn_rf_command_queue queue;
	const struct kn_rf_command_event *events[4];
	size_t count;

	if (kn_rf_command_queue_init(&queue, 4) != KN_RF_COMMAND_QUEUE_OK)
		return 1;
	if (kn_rf_command_queue_count(&queue) != 0)
		return 1;
	if (kn_rf_command_queue_list(&queue, events, 4, &count) !=
	    KN_RF_COMMAND_QUEUE_OK)
		return 1;

	return count == 0 ? 0 : 1;
}

static int
test_eviction(void)
{
	struct kn_rf_command_queue queue;
	struct kn_rf_command_event event;

	(void)kn_rf_command_queue_init(&queue, 2);
	make_event(&event, 1, "kiss0", "N0CALL");
	(void)kn_rf_command_queue_push(&queue, &event);
	make_event(&event, 2, "kiss0", "M6VPN");
	(void)kn_rf_command_queue_push(&queue, &event);
	make_event(&event, 3, "kiss0", "TEST");
	(void)kn_rf_command_queue_push(&queue, &event);

	if (kn_rf_command_queue_get(&queue, 1) != NULL)
		return 1;
	if (kn_rf_command_queue_get(&queue, 3) == NULL)
		return 1;

	return kn_rf_command_queue_count(&queue) == 2 ? 0 : 1;
}

static int
test_filters(void)
{
	struct kn_rf_command_queue queue;
	struct kn_rf_command_event event;
	const struct kn_rf_command_event *events[4];
	size_t count;

	(void)kn_rf_command_queue_init(&queue, 4);
	make_event(&event, 1, "kiss0", "N0CALL");
	(void)kn_rf_command_queue_push(&queue, &event);
	make_event(&event, 2, "kiss1", "M6VPN");
	(void)kn_rf_command_queue_push(&queue, &event);

	if (kn_rf_command_queue_list_by_port(&queue, "kiss0", events, 4,
	    &count) != KN_RF_COMMAND_QUEUE_OK || count != 1 ||
	    events[0]->id != 1)
		return 1;
	if (kn_rf_command_queue_list_by_source(&queue, "M6VPN", events, 4,
	    &count) != KN_RF_COMMAND_QUEUE_OK || count != 1 ||
	    events[0]->id != 2)
		return 1;

	return 0;
}

static int
test_push_get(void)
{
	struct kn_rf_command_queue queue;
	struct kn_rf_command_event event;
	const struct kn_rf_command_event *events[4];
	size_t count;

	(void)kn_rf_command_queue_init(&queue, 4);
	make_event(&event, 1, "kiss0", "N0CALL");
	if (kn_rf_command_queue_push(&queue, &event) !=
	    KN_RF_COMMAND_QUEUE_OK)
		return 1;
	if (kn_rf_command_queue_get(&queue, 1) == NULL)
		return 1;
	if (kn_rf_command_queue_list(&queue, events, 4, &count) !=
	    KN_RF_COMMAND_QUEUE_OK)
		return 1;

	return count == 1 && events[0]->id == 1 ? 0 : 1;
}
