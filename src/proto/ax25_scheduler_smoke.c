/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_scheduler_smoke.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_connection_key.h"
#include "kilonode/ax25_runtime.h"

static enum kn_ax25_scheduler_smoke_error smoke_create_connection(
	struct kn_ax25_runtime *, uint64_t);
static void smoke_probe_prepared_bridge(struct kn_ax25_runtime *, uint64_t);

static enum kn_ax25_scheduler_smoke_error
smoke_create_connection(struct kn_ax25_runtime *runtime, uint64_t now_ms)
{
	struct kn_ax25_connection_key key;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_params table_params;
	enum kn_ax25_connection_table_error table_rc;
	uint8_t old_allow;

	if (kn_ax25_connection_key_from_callsigns(&key, "smoke0",
	    "M6VPN-1", "N0CALL", NULL, 0) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_SCHEDULER_SMOKE_ERR_RUNTIME;
	if (kn_ax25_connection_event_local_connect(&event, now_ms,
	    &key) != KN_AX25_CONNECTION_EVENT_OK)
		return KN_AX25_SCHEDULER_SMOKE_ERR_RUNTIME;

	table_params = runtime->table.params;
	old_allow = runtime->table.params.allow_connected_mode;
	runtime->table.params.allow_connected_mode = 1;
	kn_ax25_connection_table_result_clear(&result);
	table_rc = kn_ax25_connection_table_process(&runtime->table,
	    &event, &result);
	runtime->table.params = table_params;
	if (table_rc != KN_AX25_CONNECTION_TABLE_OK)
		return KN_AX25_SCHEDULER_SMOKE_ERR_RUNTIME;
	if (result.record_index < runtime->table.count)
		runtime->table.records[result.record_index].
		    connection.params.allow_connected_mode = 1;

	runtime->counters.events_accepted++;
	if (result.created != 0) {
		runtime->counters.connections_created++;
		runtime->smoke_counters.test_connections_created++;
	}
	runtime->counters.frame_plans_generated += result.plans.count;
	(void)kn_ax25_scheduler_apply_actions(&runtime->scheduler,
	    (uint32_t)(result.record_index + 1U),
	    &runtime->table.records[result.record_index].connection.params,
	    &result.actions, now_ms);
	(void)kn_ax25_runtime_prepare_plans(runtime,
	    (uint32_t)(result.record_index + 1U), key.port_name, now_ms,
	    &result.plans);
	runtime->table.params.allow_connected_mode = old_allow;
	return KN_AX25_SCHEDULER_SMOKE_OK;
}

static void
smoke_probe_prepared_bridge(struct kn_ax25_runtime *runtime, uint64_t first_id)
{
	uint64_t id;
	uint64_t last_id;

	if (runtime == NULL)
		return;
	last_id = runtime->prepared_queue.next_id;
	for (id = first_id; id < last_id; id++)
		(void)kn_ax25_runtime_bridge_prepared_to_tx(runtime, id);
}

void
kn_ax25_scheduler_smoke_options_default(
	struct kn_ax25_scheduler_smoke_options *options)
{
	if (options == NULL)
		return;

	memset(options, 0, sizeof(*options));
}

enum kn_ax25_scheduler_smoke_error
kn_ax25_scheduler_smoke_poll(struct kn_ax25_runtime *runtime, uint64_t now_ms)
{
	uint64_t prepared_before;
	uint64_t bridge_blocked_before;
	uint64_t expired_before;
	size_t connection_count;
	enum kn_ax25_scheduler_smoke_error rc;

	if (runtime == NULL)
		return KN_AX25_SCHEDULER_SMOKE_ERR_INVALID_ARGUMENT;
	if (runtime->smoke_options.enabled == 0)
		return KN_AX25_SCHEDULER_SMOKE_OK;

	rc = kn_ax25_scheduler_smoke_validate(runtime,
	    &runtime->smoke_options);
	if (rc != KN_AX25_SCHEDULER_SMOKE_OK) {
		runtime->smoke_counters.last_error = rc;
		return rc;
	}

	runtime->smoke_counters.cycles++;
	prepared_before = runtime->prepared_queue.next_id;
	bridge_blocked_before = runtime->prepared_counters.bridge_blocked;
	connection_count = kn_ax25_runtime_connection_count(runtime);
	if (runtime->smoke_options.create_test_connection != 0 &&
	    connection_count == 0) {
		rc = smoke_create_connection(runtime, now_ms);
		if (rc != KN_AX25_SCHEDULER_SMOKE_OK) {
			runtime->smoke_counters.last_error = rc;
			return rc;
		}
	}

	expired_before = runtime->live_scheduler.expired_processed;
	runtime->smoke_counters.scheduler_polls++;
	(void)kn_ax25_live_scheduler_poll(runtime, now_ms);
	smoke_probe_prepared_bridge(runtime, prepared_before);
	runtime->smoke_counters.expired_processed +=
	    runtime->live_scheduler.expired_processed - expired_before;
	runtime->smoke_counters.prepared_frames_generated =
	    runtime->prepared_counters.frames_stored;
	runtime->smoke_counters.prepared_bridge_blocked +=
	    runtime->prepared_counters.bridge_blocked - bridge_blocked_before;
	runtime->smoke_counters.tx_writes_attempted =
	    runtime->live_scheduler.tx_writes_attempted +
	    runtime->prepared_counters.tx_queue_writes_attempted +
	    runtime->prepared_tx_counters.tx_queue_writes;
	runtime->smoke_counters.dispatch_calls_attempted = 0;
	runtime->smoke_counters.last_error = KN_AX25_SCHEDULER_SMOKE_OK;
	return KN_AX25_SCHEDULER_SMOKE_OK;
}

void
kn_ax25_scheduler_smoke_reset_counters(
	struct kn_ax25_scheduler_smoke_counters *counters)
{
	if (counters == NULL)
		return;

	memset(counters, 0, sizeof(*counters));
}

enum kn_ax25_scheduler_smoke_error
kn_ax25_scheduler_smoke_validate(const struct kn_ax25_runtime *runtime,
	const struct kn_ax25_scheduler_smoke_options *options)
{
	if (runtime == NULL || options == NULL)
		return KN_AX25_SCHEDULER_SMOKE_ERR_INVALID_ARGUMENT;
	if (options->enabled > 1 || options->create_test_connection > 1)
		return KN_AX25_SCHEDULER_SMOKE_ERR_INVALID_VALUE;
	if (options->enabled == 0 && options->create_test_connection != 0)
		return KN_AX25_SCHEDULER_SMOKE_ERR_INVALID_VALUE;
	if (options->enabled != 0 &&
	    (runtime->enabled == 0 ||
	    runtime->diagnostics_enabled == 0 ||
	    runtime->live_scheduler.policy.enabled == 0))
		return KN_AX25_SCHEDULER_SMOKE_ERR_INVALID_VALUE;

	return KN_AX25_SCHEDULER_SMOKE_OK;
}
