/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_runtime.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_rx_feed.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/callsign.h"

static int make_key(struct kn_ax25_connection_key *);
static int make_sabm_event(struct kn_ax25_connection_event_record *,
	uint64_t);
static int test_counters_update(void);
static int test_default_runtime_disabled(void);
static int test_disabled_runtime_rejects_injection(void);
static int test_enable_live_feed_validates_dependencies(void);
static int test_init_with_max_connections(void);
static int test_inject_sabm_creates_connection(void);
static int test_live_feed_defaults_disabled(void);
static int test_live_feed_updates_table_when_enabled(void);
static int test_no_tx_queue_writes(void);
static int test_params_stored(void);
static int test_prepared_bridge_blocked_counter(void);
static int test_prepared_defaults_enabled(void);
static int test_prepared_disabled_stores_none(void);
static int test_prepared_queue_full_counter(void);
static int test_prepared_raw_build_disabled(void);
static int test_reset_clears_live_counters(void);
static int test_reset_clears_live_scheduler(void);
static int test_reset_clears_prepared_queue(void);
static int test_reset_clears_scheduler(void);
static int test_reset_clears_table(void);
static int test_scheduler_policy_defaults_disabled(void);
static int test_scheduler_policy_validates_dependencies(void);

int
main(void)
{
	if (test_default_runtime_disabled() != 0)
		return 1;
	if (test_init_with_max_connections() != 0)
		return 1;
	if (test_params_stored() != 0)
		return 1;
	if (test_disabled_runtime_rejects_injection() != 0)
		return 1;
	if (test_live_feed_defaults_disabled() != 0)
		return 1;
	if (test_enable_live_feed_validates_dependencies() != 0)
		return 1;
	if (test_live_feed_updates_table_when_enabled() != 0)
		return 1;
	if (test_scheduler_policy_defaults_disabled() != 0)
		return 1;
	if (test_scheduler_policy_validates_dependencies() != 0)
		return 1;
	if (test_prepared_defaults_enabled() != 0)
		return 1;
	if (test_inject_sabm_creates_connection() != 0)
		return 1;
	if (test_counters_update() != 0)
		return 1;
	if (test_prepared_disabled_stores_none() != 0)
		return 1;
	if (test_prepared_raw_build_disabled() != 0)
		return 1;
	if (test_prepared_queue_full_counter() != 0)
		return 1;
	if (test_prepared_bridge_blocked_counter() != 0)
		return 1;
	if (test_reset_clears_table() != 0)
		return 1;
	if (test_reset_clears_live_counters() != 0)
		return 1;
	if (test_reset_clears_live_scheduler() != 0)
		return 1;
	if (test_reset_clears_prepared_queue() != 0)
		return 1;
	if (test_reset_clears_scheduler() != 0)
		return 1;
	if (test_no_tx_queue_writes() != 0)
		return 1;

	return 0;
}

static int
make_key(struct kn_ax25_connection_key *key)
{
	if (key == NULL)
		return 1;

	return kn_ax25_connection_key_from_callsigns(key, "kiss0",
	    "M6VPN-1", "N0CALL", NULL, 0) == KN_AX25_CONNECTION_KEY_OK ?
	    0 : 1;
}

static int
make_sabm_event(struct kn_ax25_connection_event_record *event, uint64_t now)
{
	struct kn_ax25_connection_key key;

	if (event == NULL || make_key(&key) != 0)
		return 1;
	if (kn_ax25_connection_event_local_connect(event, now, &key) !=
	    KN_AX25_CONNECTION_EVENT_OK)
		return 1;
	event->kind = KN_AX25_CONNECTION_EVENT_RX_SABM;
	event->control.class = KN_AX25_CONTROL_CLASS_U;
	event->control.u_subtype = KN_AX25_U_SUBTYPE_SABM;
	return 0;
}

static int
test_counters_update(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (runtime.counters.events_accepted != 1 ||
	    runtime.counters.connections_created != 1 ||
	    runtime.counters.frame_plans_generated == 0)
		return 1;

	return runtime.counters.events_rejected == 0 ? 0 : 1;
}

static int
test_default_runtime_disabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.enabled != 0 || runtime.connected_mode_enabled != 0)
		return 1;
	if (runtime.diagnostics_enabled == 0)
		return 1;
	if (kn_ax25_runtime_connection_count(&runtime) != 0)
		return 1;

	return runtime.max_connections == KN_AX25_CONNECTION_TABLE_DEFAULT_MAX ?
	    0 : 1;
}

static int
test_disabled_runtime_rejects_injection(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(&runtime);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_ERR_DISABLED)
		return 1;
	if (kn_ax25_runtime_connection_count(&runtime) != 0)
		return 1;

	return runtime.counters.events_rejected == 1 ? 0 : 1;
}

static int
test_enable_live_feed_validates_dependencies(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_live_options live;

	kn_ax25_runtime_init(&runtime);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	if (kn_ax25_runtime_set_live_options(&runtime, &live) !=
	    KN_AX25_RUNTIME_ERR_INVALID_VALUE)
		return 1;
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	live.live_rx_create_connections = 1;
	if (kn_ax25_runtime_set_live_options(&runtime, &live) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return runtime.live.live_rx_create_connections == 1 ? 0 : 1;
}

static int
test_init_with_max_connections(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_params params;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	if (kn_ax25_runtime_set_params(&runtime, &params, 4) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (runtime.max_connections != 4)
		return 1;

	return runtime.table.max_connections == 4 ? 0 : 1;
}

static int
test_inject_sabm_creates_connection(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;
	const struct kn_ax25_connection_record *record;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	record = kn_ax25_runtime_get_connection(&runtime, result.record_index);
	if (record == NULL || result.created == 0)
		return 1;
	if (record->last_plans.count == 0)
		return 1;

	return kn_ax25_runtime_connection_count(&runtime) == 1 ? 0 : 1;
}

static int
test_live_feed_defaults_disabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.live.live_rx_feed != 0)
		return 1;
	if (runtime.live.live_rx_create_connections != 0)
		return 1;

	return runtime.live.live_rx_retain_frame_plans == 1 ? 0 : 1;
}

static int
test_live_feed_updates_table_when_enabled(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	struct kn_ax25_live_options live;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	live.live_rx_create_connections = 1;
	live.live_rx_retain_frame_plans = 1;
	(void)kn_ax25_runtime_set_live_options(&runtime, &live);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	kn_ax25_frame_reset(&frame);
	(void)kn_callsign_parse("M6VPN-1", &frame.destination.callsign);
	(void)kn_callsign_parse("N0CALL", &frame.source.callsign);
	frame.control = 0x2f;
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, NULL) != KN_AX25_RX_FEED_OK)
		return 1;

	return kn_ax25_runtime_connection_count(&runtime) == 1 &&
	    runtime.live_counters.frames_accepted == 1 ? 0 : 1;
}

static int
test_scheduler_policy_defaults_disabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.live_scheduler.policy.enabled != 0)
		return 1;
	if (runtime.live_scheduler.policy.process_expired != 0)
		return 1;

	return runtime.live_scheduler.policy.max_expired_per_cycle ==
	    KN_AX25_LIVE_SCHEDULER_EXPIRED_DEFAULT ? 0 : 1;
}

static int
test_scheduler_policy_validates_dependencies(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_ERR_INVALID_VALUE)
		return 1;
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	policy.tx_actions_enabled = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_ERR_INVALID_VALUE)
		return 1;
	policy.tx_actions_enabled = 0;
	policy.process_expired = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return runtime.live_scheduler.policy.process_expired == 1 ? 0 : 1;
}

static int
test_no_tx_queue_writes(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return result.plans.count > 0 &&
	    runtime.counters.frame_plans_generated == result.plans.count ?
	    0 : 1;
}

static int
test_params_stored(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_params params;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	params.t1_ms = 4000;
	if (kn_ax25_runtime_set_params(&runtime, &params, 8) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return runtime.params.t1_ms == 4000 &&
	    runtime.connected_mode_enabled == 1 ? 0 : 1;
}

static int
test_prepared_defaults_enabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.prepared_policy.enabled == 0 ||
	    runtime.prepared_policy.build_raw == 0)
		return 1;

	return runtime.prepared_queue.max_frames ==
	    KN_AX25_PREPARED_QUEUE_DEFAULT_MAX ? 0 : 1;
}

static int
test_prepared_bridge_blocked_counter(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_runtime_bridge_prepared_to_tx(&runtime, 1) !=
	    KN_AX25_RUNTIME_ERR_DISABLED)
		return 1;

	return runtime.prepared_counters.bridge_blocked == 1 &&
	    runtime.prepared_counters.tx_queue_writes_attempted == 0 ? 0 : 1;
}

static int
test_prepared_disabled_stores_none(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_prepared_policy policy;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_prepared_policy_default(&policy);
	policy.enabled = 0;
	if (kn_ax25_runtime_set_prepared_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return runtime.prepared_queue.count == 0 &&
	    runtime.prepared_counters.frames_attempted == 0 ? 0 : 1;
}

static int
test_prepared_queue_full_counter(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_prepared_policy policy;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_prepared_policy_default(&policy);
	policy.max_frames = 1;
	if (kn_ax25_runtime_set_prepared_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_runtime_prepare_plans(&runtime, 1, "kiss0", 1710000001,
	    &result.plans) != KN_AX25_RUNTIME_OK)
		return 1;

	return runtime.prepared_counters.queue_full == 1 &&
	    runtime.prepared_counters.tx_queue_writes_attempted == 0 ? 0 : 1;
}

static int
test_prepared_raw_build_disabled(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_prepared_policy policy;
	const struct kn_ax25_prepared_frame *frame;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_prepared_policy_default(&policy);
	policy.build_raw = 0;
	if (kn_ax25_runtime_set_prepared_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	frame = kn_ax25_prepared_queue_get(&runtime.prepared_queue, 1);
	if (frame == NULL)
		return 1;

	return frame->raw_len == 0 &&
	    runtime.prepared_counters.frames_stored == 1 ? 0 : 1;
}

static int
test_reset_clears_live_counters(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	runtime.live_counters.frames_seen = 2;
	kn_ax25_runtime_reset(&runtime);

	return runtime.live_counters.frames_seen == 0 &&
	    runtime.live.live_rx_retain_frame_plans == 1 ? 0 : 1;
}

static int
test_reset_clears_live_scheduler(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	runtime.live_scheduler.policy.enabled = 1;
	runtime.live_scheduler.cycles_run = 5;
	runtime.live_scheduler.expired_processed = 2;
	kn_ax25_runtime_reset(&runtime);
	if (runtime.live_scheduler.policy.enabled != 1)
		return 1;

	return runtime.live_scheduler.cycles_run == 0 &&
	    runtime.live_scheduler.expired_processed == 0 ? 0 : 1;
}

static int
test_reset_clears_prepared_queue(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (runtime.prepared_queue.count == 0)
		return 1;
	kn_ax25_runtime_reset(&runtime);

	return runtime.prepared_queue.count == 0 &&
	    runtime.prepared_policy.enabled == 1 ? 0 : 1;
}

static int
test_reset_clears_scheduler(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_timer_queue_start(&runtime.scheduler.queue, 1,
	    KN_AX25_TIMER_T1, 0, 100);
	runtime.scheduler.counters.timers_started = 1;
	kn_ax25_runtime_reset(&runtime);
	if (runtime.scheduler.queue.count != 0)
		return 1;

	return runtime.scheduler.counters.timers_started == 0 ? 0 : 1;
}

static int
test_reset_clears_table(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 1);
	if (make_sabm_event(&event, 1710000000) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(&runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	kn_ax25_runtime_reset(&runtime);
	if (kn_ax25_runtime_connection_count(&runtime) != 0)
		return 1;

	return runtime.enabled == 1 && runtime.connected_mode_enabled == 1 ?
	    0 : 1;
}
