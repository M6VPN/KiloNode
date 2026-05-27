/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_queue.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_prepared_queue.h"
#include "kilonode/callsign.h"

static int make_frame(struct kn_ax25_prepared_frame *, const char *,
	uint32_t);
static int test_empty_queue(void);
static int test_list_filters(void);
static int test_push_get(void);
static int test_queue_full(void);
static int test_reset(void);

int
main(void)
{
	if (test_empty_queue() != 0)
		return 1;
	if (test_push_get() != 0)
		return 1;
	if (test_list_filters() != 0)
		return 1;
	if (test_queue_full() != 0)
		return 1;
	if (test_reset() != 0)
		return 1;

	return 0;
}

static int
make_frame(struct kn_ax25_prepared_frame *frame, const char *port,
	uint32_t connection_id)
{
	if (frame == NULL || port == NULL)
		return 1;

	kn_ax25_prepared_frame_clear(frame);
	frame->connection_id = connection_id;
	if (snprintf(frame->port_name, sizeof(frame->port_name), "%s",
	    port) < 0)
		return 1;
	if (kn_callsign_parse("M6VPN-1", &frame->local) != 0 ||
	    kn_callsign_parse("N0CALL", &frame->remote) != 0)
		return 1;
	frame->type = KN_AX25_FRAME_PLAN_UA;
	frame->action_source = KN_AX25_ACTION_SEND_UA;
	frame->status = KN_AX25_PREPARED_FRAME_STATUS_PREPARED;
	return 0;
}

static int
test_empty_queue(void)
{
	struct kn_ax25_prepared_queue queue;

	kn_ax25_prepared_queue_init(&queue);

	return kn_ax25_prepared_queue_count(&queue) == 0 &&
	    kn_ax25_prepared_queue_get(&queue, 1) == NULL ? 0 : 1;
}

static int
test_list_filters(void)
{
	struct kn_ax25_prepared_queue queue;
	struct kn_ax25_prepared_frame frame;
	const struct kn_ax25_prepared_frame *frames[4];
	size_t count;
	uint64_t id;

	kn_ax25_prepared_queue_init(&queue);
	if (make_frame(&frame, "kiss0", 1) != 0 ||
	    kn_ax25_prepared_queue_push(&queue, &frame, &id) !=
	    KN_AX25_PREPARED_QUEUE_OK)
		return 1;
	if (make_frame(&frame, "kiss1", 2) != 0 ||
	    kn_ax25_prepared_queue_push(&queue, &frame, &id) !=
	    KN_AX25_PREPARED_QUEUE_OK)
		return 1;
	if (kn_ax25_prepared_queue_list_by_port(&queue, "kiss0", frames,
	    4, &count) != KN_AX25_PREPARED_QUEUE_OK || count != 1)
		return 1;
	if (kn_ax25_prepared_queue_list_by_connection(&queue, 2, frames,
	    4, &count) != KN_AX25_PREPARED_QUEUE_OK || count != 1)
		return 1;

	return frames[0]->connection_id == 2 ? 0 : 1;
}

static int
test_push_get(void)
{
	struct kn_ax25_prepared_queue queue;
	struct kn_ax25_prepared_frame frame;
	const struct kn_ax25_prepared_frame *stored;
	uint64_t id;

	kn_ax25_prepared_queue_init(&queue);
	if (make_frame(&frame, "kiss0", 1) != 0)
		return 1;
	if (kn_ax25_prepared_queue_push(&queue, &frame, &id) !=
	    KN_AX25_PREPARED_QUEUE_OK)
		return 1;
	stored = kn_ax25_prepared_queue_get(&queue, id);
	if (stored == NULL || stored->id != 1)
		return 1;

	return queue.next_id == 2 ? 0 : 1;
}

static int
test_queue_full(void)
{
	struct kn_ax25_prepared_queue queue;
	struct kn_ax25_prepared_frame frame;
	uint64_t id;

	kn_ax25_prepared_queue_init(&queue);
	if (kn_ax25_prepared_queue_set_max(&queue, 1) !=
	    KN_AX25_PREPARED_QUEUE_OK)
		return 1;
	if (make_frame(&frame, "kiss0", 1) != 0)
		return 1;
	if (kn_ax25_prepared_queue_push(&queue, &frame, &id) !=
	    KN_AX25_PREPARED_QUEUE_OK)
		return 1;

	return kn_ax25_prepared_queue_push(&queue, &frame, &id) ==
	    KN_AX25_PREPARED_QUEUE_ERR_FULL ? 0 : 1;
}

static int
test_reset(void)
{
	struct kn_ax25_prepared_queue queue;
	struct kn_ax25_prepared_frame frame;
	uint64_t id;

	kn_ax25_prepared_queue_init(&queue);
	if (make_frame(&frame, "kiss0", 1) != 0 ||
	    kn_ax25_prepared_queue_push(&queue, &frame, &id) !=
	    KN_AX25_PREPARED_QUEUE_OK)
		return 1;
	kn_ax25_prepared_queue_reset(&queue);

	return queue.count == 0 && queue.next_id == 1 ? 0 : 1;
}
