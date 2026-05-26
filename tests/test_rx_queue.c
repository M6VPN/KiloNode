/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rx_queue.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/callsign.h"
#include "kilonode/rx_queue.h"

static void make_event(struct kn_rx_event *, uint64_t, const char *,
	const char *, const char *);
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
make_event(struct kn_rx_event *event, uint64_t id, const char *port,
	const char *source, const char *destination)
{
	kn_rx_event_clear(event);
	event->id = id;
	(void)snprintf(event->port_name, sizeof(event->port_name), "%s", port);
	(void)kn_callsign_parse(source, &event->source);
	(void)kn_callsign_parse(destination, &event->destination);
	event->kind = KN_RX_FRAME_UI;
}

static int
test_empty(void)
{
	struct kn_rx_queue queue;
	const struct kn_rx_event *events[4];
	size_t count;

	kn_rx_queue_init(&queue, 4, 80, 1);
	if (kn_rx_queue_count(&queue) != 0)
		return 1;
	if (kn_rx_queue_newest(&queue) != NULL)
		return 1;
	if (kn_rx_queue_list(&queue, events, 4, &count) != KN_RX_QUEUE_OK)
		return 1;

	return count == 0 ? 0 : 1;
}

static int
test_eviction(void)
{
	struct kn_rx_queue queue;
	struct kn_rx_event event;

	kn_rx_queue_init(&queue, 2, 80, 1);
	make_event(&event, 1, "kiss0", "M6VPN-1", "CQ");
	(void)kn_rx_queue_push(&queue, &event);
	make_event(&event, 2, "kiss0", "N0CALL", "CQ");
	(void)kn_rx_queue_push(&queue, &event);
	make_event(&event, 3, "kiss0", "TEST", "CQ");
	(void)kn_rx_queue_push(&queue, &event);

	if (kn_rx_queue_get(&queue, 1) != NULL)
		return 1;
	if (kn_rx_queue_get(&queue, 3) == NULL)
		return 1;

	return kn_rx_queue_count(&queue) == 2 ? 0 : 1;
}

static int
test_filters(void)
{
	struct kn_rx_queue queue;
	struct kn_rx_event event;
	struct kn_callsign callsign;
	const struct kn_rx_event *events[4];
	size_t count;

	kn_rx_queue_init(&queue, 4, 80, 1);
	make_event(&event, 1, "kiss0", "M6VPN-1", "CQ");
	(void)kn_rx_queue_push(&queue, &event);
	make_event(&event, 2, "kiss1", "N0CALL", "APRS");
	(void)kn_rx_queue_push(&queue, &event);

	if (kn_rx_queue_list_by_port(&queue, "kiss0", events, 4,
	    &count) != KN_RX_QUEUE_OK || count != 1 || events[0]->id != 1)
		return 1;
	(void)kn_callsign_parse("N0CALL", &callsign);
	if (kn_rx_queue_list_by_source(&queue, &callsign, events, 4,
	    &count) != KN_RX_QUEUE_OK || count != 1 || events[0]->id != 2)
		return 1;
	(void)kn_callsign_parse("CQ", &callsign);
	if (kn_rx_queue_list_by_destination(&queue, &callsign, events, 4,
	    &count) != KN_RX_QUEUE_OK || count != 1 || events[0]->id != 1)
		return 1;

	return 0;
}

static int
test_push_get(void)
{
	struct kn_rx_queue queue;
	struct kn_rx_event event;
	const struct kn_rx_event *events[4];
	size_t count;

	kn_rx_queue_init(&queue, 4, 80, 1);
	make_event(&event, 1, "kiss0", "M6VPN-1", "CQ");
	if (kn_rx_queue_push(&queue, &event) != KN_RX_QUEUE_OK)
		return 1;
	if (kn_rx_queue_get(&queue, 1) == NULL)
		return 1;
	if (kn_rx_queue_list(&queue, events, 4, &count) != KN_RX_QUEUE_OK)
		return 1;

	return count == 1 && events[0]->id == 1 ? 0 : 1;
}
