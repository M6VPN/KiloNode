/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_tx_queue.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/tx_queue.h"

static void make_frame(struct kn_tx_frame *, uint64_t, const char *);
static int test_empty(void);
static int test_full_rejects(void);
static int test_mark_status(void);
static int test_push_get_list(void);

int
main(void)
{
	if (test_empty() != 0)
		return 1;
	if (test_push_get_list() != 0)
		return 1;
	if (test_full_rejects() != 0)
		return 1;
	if (test_mark_status() != 0)
		return 1;

	return 0;
}

static void
make_frame(struct kn_tx_frame *frame, uint64_t id, const char *port)
{
	kn_tx_frame_clear(frame);
	frame->id = id;
	(void)snprintf(frame->port_name, sizeof(frame->port_name), "%s", port);
	frame->kind = KN_TX_FRAME_UI;
	frame->status = KN_TX_FRAME_QUEUED;
}

static int
test_empty(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	const struct kn_tx_frame *frames[4];
	size_t count;

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.max_queued = 4;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	if (kn_tx_queue_count(&queue) != 0)
		return 1;
	if (kn_tx_queue_list(&queue, frames, 4, &count) != KN_TX_QUEUE_OK)
		return 1;

	return count == 0 ? 0 : 1;
}

static int
test_full_rejects(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_frame frame;

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.max_queued = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&frame, 1, "kiss0");
	if (kn_tx_queue_enqueue(&queue, &frame) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&frame, 2, "kiss0");

	return kn_tx_queue_enqueue(&queue, &frame) == KN_TX_QUEUE_ERR_FULL ?
	    0 : 1;
}

static int
test_mark_status(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_frame frame;
	const struct kn_tx_frame *stored;

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.max_queued = 4;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&frame, 1, "kiss0");
	if (kn_tx_queue_enqueue(&queue, &frame) != KN_TX_QUEUE_OK)
		return 1;
	if (kn_tx_queue_mark_sent(&queue, 1) != KN_TX_QUEUE_OK)
		return 1;
	stored = kn_tx_queue_get(&queue, 1);
	if (stored == NULL || stored->status != KN_TX_FRAME_SENT)
		return 1;
	if (kn_tx_queue_mark_dropped(&queue, 1) != KN_TX_QUEUE_OK)
		return 1;
	if (kn_tx_queue_mark_failed(&queue, 1, 5) != KN_TX_QUEUE_OK)
		return 1;
	stored = kn_tx_queue_get(&queue, 1);

	return stored != NULL && stored->status == KN_TX_FRAME_FAILED &&
	    stored->last_error == 5 ? 0 : 1;
}

static int
test_push_get_list(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_frame frame;
	const struct kn_tx_frame *frames[4];
	size_t count;

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.max_queued = 4;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&frame, kn_tx_queue_reserve_id(&queue), "kiss0");
	if (kn_tx_queue_enqueue(&queue, &frame) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&frame, kn_tx_queue_reserve_id(&queue), "kiss1");
	if (kn_tx_queue_enqueue(&queue, &frame) != KN_TX_QUEUE_OK)
		return 1;
	if (kn_tx_queue_get(&queue, 1) == NULL)
		return 1;
	if (kn_tx_queue_list_by_port(&queue, "kiss1", frames, 4,
	    &count) != KN_TX_QUEUE_OK)
		return 1;
	if (count != 1 || frames[0]->id != 2)
		return 1;

	return kn_tx_queue_pop_next_for_port(&queue, "kiss0") != NULL ?
	    0 : 1;
}
