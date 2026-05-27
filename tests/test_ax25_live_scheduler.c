/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_live_scheduler.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_live_scheduler.h"
#include "kilonode/ax25_runtime.h"

static int make_event(struct kn_ax25_connection_event_record *,
	const char *, uint64_t);
static int setup_connection(struct kn_ax25_runtime *, const char *,
	uint64_t);
static int test_frame_plans_retained_only(void);
static int test_init_disabled(void);
static int test_max_expired_per_cycle(void);
static int test_poll_disabled_noop(void);
static int test_poll_process_expired_false_blocks(void);
static int test_poll_processes_t1(void);
static int test_reset_clears_counters(void);

int
main(void)
{
	if (test_init_disabled() != 0)
		return 1;
	if (test_poll_disabled_noop() != 0)
		return 1;
	if (test_poll_process_expired_false_blocks() != 0)
		return 1;
	if (test_poll_processes_t1() != 0)
		return 1;
	if (test_max_expired_per_cycle() != 0)
		return 1;
	if (test_frame_plans_retained_only() != 0)
		return 1;
	if (test_reset_clears_counters() != 0)
		return 1;

	return 0;
}

static int
make_event(struct kn_ax25_connection_event_record *event,
	const char *remote, uint64_t now)
{
	struct kn_ax25_connection_key key;

	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", remote, NULL, 0) != KN_AX25_CONNECTION_KEY_OK)
		return 1;
	if (kn_ax25_connection_event_local_connect(event, now, &key) !=
	    KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return 0;
}

static int
setup_connection(struct kn_ax25_runtime *runtime, const char *remote,
	uint64_t now)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;
	uint32_t id;

	if (make_event(&event, remote, now) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	id = (uint32_t)result.record_index + 1U;
	if (kn_ax25_scheduler_apply_actions(&runtime->scheduler, id,
	    &runtime->params, &result.actions, now) != KN_AX25_SCHEDULER_OK)
		return 1;

	return 0;
}

static int
test_frame_plans_retained_only(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_policy policy;
	const struct kn_ax25_connection_record *record;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (setup_connection(&runtime, "N0CALL", 0) != 0)
		return 1;
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	policy.process_expired = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_live_scheduler_poll(&runtime, 3000) !=
	    KN_AX25_LIVE_SCHEDULER_OK)
		return 1;
	record = kn_ax25_runtime_get_connection(&runtime, 0);
	if (record == NULL || record->last_plans.count == 0)
		return 1;

	return runtime.live_scheduler.tx_writes_attempted == 0 &&
	    runtime.scheduler.counters.tx_queue_writes_attempted == 0 ? 0 : 1;
}

static int
test_init_disabled(void)
{
	struct kn_ax25_live_scheduler live;

	kn_ax25_live_scheduler_init(&live);
	if (live.policy.enabled != 0 || live.policy.process_expired != 0)
		return 1;

	return live.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_max_expired_per_cycle(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (setup_connection(&runtime, "N0CALL", 0) != 0)
		return 1;
	if (setup_connection(&runtime, "N1CALL", 0) != 0)
		return 1;
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	policy.process_expired = 1;
	policy.max_expired_per_cycle = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_live_scheduler_poll(&runtime, 3000) !=
	    KN_AX25_LIVE_SCHEDULER_OK)
		return 1;

	return runtime.live_scheduler.expired_processed == 1 &&
	    kn_ax25_timer_queue_count_running(&runtime.scheduler.queue) == 2 ?
	    0 : 1;
}

static int
test_poll_disabled_noop(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_live_scheduler_poll(&runtime, 100) !=
	    KN_AX25_LIVE_SCHEDULER_OK)
		return 1;

	return runtime.live_scheduler.cycles_run == 0 ? 0 : 1;
}

static int
test_poll_process_expired_false_blocks(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (setup_connection(&runtime, "N0CALL", 0) != 0)
		return 1;
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_live_scheduler_poll(&runtime, 3000) !=
	    KN_AX25_LIVE_SCHEDULER_OK)
		return 1;

	return runtime.live_scheduler.expired_blocked_by_policy == 1 &&
	    runtime.live_scheduler.expired_processed == 0 ? 0 : 1;
}

static int
test_poll_processes_t1(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (setup_connection(&runtime, "N0CALL", 0) != 0)
		return 1;
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	policy.process_expired = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_live_scheduler_poll(&runtime, 3000) !=
	    KN_AX25_LIVE_SCHEDULER_OK)
		return 1;
	if (runtime.live_scheduler.expired_processed != 1)
		return 1;
	if (runtime.live_scheduler.tx_actions_blocked == 0)
		return 1;

	return runtime.live_scheduler.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_reset_clears_counters(void)
{
	struct kn_ax25_live_scheduler live;

	kn_ax25_live_scheduler_init(&live);
	live.policy.enabled = 1;
	live.cycles_run = 2;
	live.expired_processed = 3;
	kn_ax25_live_scheduler_reset(&live);
	if (live.policy.enabled != 1)
		return 1;

	return live.cycles_run == 0 && live.expired_processed == 0 ? 0 : 1;
}
