/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_scheduler.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25_scheduler.h"

static int action_has(const struct kn_ax25_action_list *,
	enum kn_ax25_action_intent);
static void enabled_params(struct kn_ax25_params *);
static int test_apply_retry_actions(void);
static int test_apply_timer_actions(void);
static int test_format(void);
static int test_init_empty(void);
static int test_no_tx_writes_for_send_actions(void);
static int test_process_t1_expiry(void);
static int test_retry_exhaustion_counter(void);
static int test_t2_placeholder(void);
static int test_t3_expiry_event(void);

int
main(void)
{
	if (test_init_empty() != 0)
		return 1;
	if (test_apply_timer_actions() != 0)
		return 1;
	if (test_apply_retry_actions() != 0)
		return 1;
	if (test_no_tx_writes_for_send_actions() != 0)
		return 1;
	if (test_process_t1_expiry() != 0)
		return 1;
	if (test_retry_exhaustion_counter() != 0)
		return 1;
	if (test_t3_expiry_event() != 0)
		return 1;
	if (test_t2_placeholder() != 0)
		return 1;
	if (test_format() != 0)
		return 1;

	return 0;
}

static int
action_has(const struct kn_ax25_action_list *actions,
	enum kn_ax25_action_intent intent)
{
	size_t i;

	for (i = 0; i < actions->count; i++) {
		if (actions->actions[i].intent == intent)
			return 1;
	}

	return 0;
}

static void
enabled_params(struct kn_ax25_params *params)
{
	kn_ax25_params_default(params);
	params->allow_connected_mode = 1;
	params->t1_ms = 100;
	params->t2_ms = 50;
	params->t3_ms = 1000;
	params->n2_retry_count = 3;
}

static int
test_apply_retry_actions(void)
{
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_params params;
	struct kn_ax25_action_list actions;

	kn_ax25_scheduler_init(&scheduler);
	enabled_params(&params);
	kn_ax25_action_list_clear(&actions);
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_RESET_RETRY_COUNT) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_INCREMENT_RETRY_COUNT) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_scheduler_apply_actions(&scheduler, 1, &params,
	    &actions, 0) != KN_AX25_SCHEDULER_OK)
		return 1;
	if (scheduler.connection_count != 1)
		return 1;

	return scheduler.counters.retries_incremented == 1 ? 0 : 1;
}

static int
test_apply_timer_actions(void)
{
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_params params;
	struct kn_ax25_action_list actions;

	kn_ax25_scheduler_init(&scheduler);
	enabled_params(&params);
	kn_ax25_action_list_clear(&actions);
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_START_T1) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_START_T3) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_scheduler_apply_actions(&scheduler, 5, &params,
	    &actions, 10) != KN_AX25_SCHEDULER_OK)
		return 1;
	if (kn_ax25_timer_queue_count_running(&scheduler.queue) != 2)
		return 1;
	kn_ax25_action_list_clear(&actions);
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_STOP_T1) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_scheduler_apply_actions(&scheduler, 5, &params,
	    &actions, 20) != KN_AX25_SCHEDULER_OK)
		return 1;

	return scheduler.counters.timers_started == 2 &&
	    scheduler.counters.timers_stopped == 1 ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_scheduler scheduler;
	char buf[160];

	kn_ax25_scheduler_init(&scheduler);
	if (kn_ax25_scheduler_format(&scheduler, buf, sizeof(buf)) !=
	    KN_AX25_SCHEDULER_OK)
		return 1;

	return strstr(buf, "connections=0 timers=0 running=0") != NULL ?
	    0 : 1;
}

static int
test_init_empty(void)
{
	struct kn_ax25_scheduler scheduler;
	uint64_t next;

	kn_ax25_scheduler_init(&scheduler);
	if (scheduler.connection_count != 0)
		return 1;
	if (scheduler.counters.tx_queue_writes_attempted != 0)
		return 1;

	return kn_ax25_scheduler_next_wakeup(&scheduler, &next) ==
	    KN_AX25_SCHEDULER_ERR_NOT_FOUND ? 0 : 1;
}

static int
test_no_tx_writes_for_send_actions(void)
{
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_params params;
	struct kn_ax25_action_list actions;

	kn_ax25_scheduler_init(&scheduler);
	enabled_params(&params);
	kn_ax25_action_list_clear(&actions);
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_SEND_SABM) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_SEND_DISC) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_scheduler_apply_actions(&scheduler, 1, &params,
	    &actions, 0) != KN_AX25_SCHEDULER_OK)
		return 1;

	return scheduler.counters.tx_queue_writes_attempted == 0 &&
	    kn_ax25_timer_queue_count_running(&scheduler.queue) == 0 ? 0 : 1;
}

static int
test_process_t1_expiry(void)
{
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_connection connection;
	struct kn_ax25_params params;
	struct kn_ax25_scheduler_expiry_result result;

	kn_ax25_scheduler_init(&scheduler);
	enabled_params(&params);
	kn_ax25_connection_init(&connection, &params);
	connection.state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
	if (kn_ax25_scheduler_process_expiry(&scheduler, 3,
	    KN_AX25_TIMER_T1, &connection, 200, &result) !=
	    KN_AX25_SCHEDULER_OK)
		return 1;
	if (result.event != KN_AX25_CONNECTION_EVENT_TIMEOUT_T1)
		return 1;
	if (connection.state != KN_AX25_CONNECTION_AWAITING_CONNECTION)
		return 1;
	if (action_has(&result.actions, KN_AX25_ACTION_SEND_SABM) == 0)
		return 1;

	return kn_ax25_timer_queue_count_running(&scheduler.queue) == 1 &&
	    scheduler.counters.tx_queue_writes_attempted == 0 ? 0 : 1;
}

static int
test_retry_exhaustion_counter(void)
{
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_params params;
	struct kn_ax25_action_list actions;

	kn_ax25_scheduler_init(&scheduler);
	enabled_params(&params);
	params.n2_retry_count = 1;
	kn_ax25_action_list_clear(&actions);
	if (kn_ax25_action_list_append(&actions,
	    KN_AX25_ACTION_INCREMENT_RETRY_COUNT) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_scheduler_apply_actions(&scheduler, 9, &params,
	    &actions, 0) != KN_AX25_SCHEDULER_OK)
		return 1;

	return scheduler.counters.retries_exhausted == 1 ? 0 : 1;
}

static int
test_t2_placeholder(void)
{
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_timer_expiry expired[1];
	size_t count;

	kn_ax25_scheduler_init(&scheduler);
	if (kn_ax25_timer_queue_start(&scheduler.queue, 1, KN_AX25_TIMER_T2,
	    0, 10) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_scheduler_poll_expired(&scheduler, 10, expired, 1,
	    &count) != KN_AX25_SCHEDULER_OK)
		return 1;
	if (count != 1)
		return 1;

	return expired[0].planned != 0 &&
	    expired[0].event == KN_AX25_CONNECTION_EVENT_NONE ? 0 : 1;
}

static int
test_t3_expiry_event(void)
{
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_timer_expiry expired[1];
	size_t count;

	kn_ax25_scheduler_init(&scheduler);
	if (kn_ax25_timer_queue_start(&scheduler.queue, 1, KN_AX25_TIMER_T3,
	    0, 10) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_scheduler_poll_expired(&scheduler, 10, expired, 1,
	    &count) != KN_AX25_SCHEDULER_OK)
		return 1;
	if (count != 1)
		return 1;

	return expired[0].event == KN_AX25_CONNECTION_EVENT_TIMEOUT_T3 ? 0 :
	    1;
}
