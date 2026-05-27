/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_timer_queue.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25_timer_queue.h"

static int test_add_find_stop(void);
static int test_collect_multiple_ordered(void);
static int test_empty_queue(void);
static int test_format(void);
static int test_full_rejected(void);
static int test_next_expiry(void);
static int test_reset(void);

int
main(void)
{
	if (test_empty_queue() != 0)
		return 1;
	if (test_add_find_stop() != 0)
		return 1;
	if (test_collect_multiple_ordered() != 0)
		return 1;
	if (test_next_expiry() != 0)
		return 1;
	if (test_full_rejected() != 0)
		return 1;
	if (test_reset() != 0)
		return 1;
	if (test_format() != 0)
		return 1;

	return 0;
}

static int
test_add_find_stop(void)
{
	struct kn_ax25_timer_queue queue;
	size_t index;

	kn_ax25_timer_queue_init(&queue);
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T1, 0,
	    100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T2, 0,
	    50) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T3, 0,
	    200) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_find(&queue, 1, KN_AX25_TIMER_T2,
	    &index) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_stop(&queue, 1, KN_AX25_TIMER_T2) !=
	    KN_AX25_TIMER_QUEUE_OK)
		return 1;

	return kn_ax25_timer_queue_count_running(&queue) == 2 ? 0 : 1;
}

static int
test_collect_multiple_ordered(void)
{
	struct kn_ax25_timer_queue queue;
	struct kn_ax25_timer_expiry expired[3];
	size_t count;

	kn_ax25_timer_queue_init(&queue);
	if (kn_ax25_timer_queue_start(&queue, 2, KN_AX25_TIMER_T1, 0,
	    100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T3, 0,
	    100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T2, 0,
	    100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_collect_expired(&queue, 100, expired,
	    3, &count) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (count != 3)
		return 1;
	if (expired[0].connection_id != 1 ||
	    expired[0].kind != KN_AX25_TIMER_T2)
		return 1;
	if (expired[0].planned == 0)
		return 1;
	if (expired[1].connection_id != 1 ||
	    expired[1].kind != KN_AX25_TIMER_T3)
		return 1;
	if (expired[2].connection_id != 2 ||
	    expired[2].kind != KN_AX25_TIMER_T1)
		return 1;

	return kn_ax25_timer_queue_count_running(&queue) == 0 ? 0 : 1;
}

static int
test_empty_queue(void)
{
	struct kn_ax25_timer_queue queue;
	uint64_t next;

	kn_ax25_timer_queue_init(&queue);
	if (queue.count != 0)
		return 1;

	return kn_ax25_timer_queue_next_expiry(&queue, &next) ==
	    KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_timer_queue queue;
	char buf[64];

	kn_ax25_timer_queue_init(&queue);
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T1, 0,
	    100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_format(&queue, buf, sizeof(buf)) !=
	    KN_AX25_TIMER_QUEUE_OK)
		return 1;

	return strstr(buf, "timers=1 running=1") != NULL ? 0 : 1;
}

static int
test_full_rejected(void)
{
	struct kn_ax25_timer_queue queue;
	size_t i;

	kn_ax25_timer_queue_init(&queue);
	for (i = 0; i < KN_AX25_TIMER_QUEUE_MAX; i++) {
		if (kn_ax25_timer_queue_start(&queue, (uint32_t)i,
		    KN_AX25_TIMER_T1, 0, 100) != KN_AX25_TIMER_QUEUE_OK)
			return 1;
	}

	return kn_ax25_timer_queue_start(&queue, 1000, KN_AX25_TIMER_T1,
	    0, 100) == KN_AX25_TIMER_QUEUE_ERR_FULL ? 0 : 1;
}

static int
test_next_expiry(void)
{
	struct kn_ax25_timer_queue queue;
	uint64_t next;

	kn_ax25_timer_queue_init(&queue);
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T1, 50,
	    500) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_start(&queue, 2, KN_AX25_TIMER_T3, 50,
	    100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_timer_queue_next_expiry(&queue, &next) !=
	    KN_AX25_TIMER_QUEUE_OK)
		return 1;

	return next == 150 ? 0 : 1;
}

static int
test_reset(void)
{
	struct kn_ax25_timer_queue queue;

	kn_ax25_timer_queue_init(&queue);
	if (kn_ax25_timer_queue_start(&queue, 1, KN_AX25_TIMER_T1, 0,
	    100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	kn_ax25_timer_queue_reset(&queue);

	return queue.count == 0 ? 0 : 1;
}
