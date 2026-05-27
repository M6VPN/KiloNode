/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_runtime.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_prepared_frame.h"
#include "kilonode/ax25_prepared_tx_gate.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/tx_policy.h"

static void runtime_apply_table_params(struct kn_ax25_runtime *);

static void
runtime_apply_table_params(struct kn_ax25_runtime *runtime)
{
	struct kn_ax25_params params;

	if (runtime == NULL)
		return;

	params = runtime->params;
	if (runtime->live.live_rx_feed != 0)
		params.allow_connected_mode = 1;
	(void)kn_ax25_connection_table_set_params(&runtime->table, &params);
}

enum kn_ax25_runtime_error
kn_ax25_runtime_bridge_prepared_to_tx(struct kn_ax25_runtime *runtime,
	uint64_t id)
{
	const struct kn_ax25_prepared_frame *frame;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;
	struct kn_tx_policy tx_policy;

	if (runtime == NULL || id == 0)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	frame = kn_ax25_prepared_queue_get(&runtime->prepared_queue, id);
	if (frame == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	kn_tx_policy_defaults(&tx_policy);
	memset(&input, 0, sizeof(input));
	input.prepared = frame;
	input.policy = &runtime->prepared_tx_policy;
	input.tx_policy = &tx_policy;
	runtime->prepared_tx_counters.checks++;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);
	if (decision.allowed != 0)
		runtime->prepared_tx_counters.allowed++;
	else
		runtime->prepared_tx_counters.blocked++;
	if (decision.reason == KN_AX25_PREPARED_TX_REASON_FX25_NOT_SUPPORTED)
		runtime->prepared_tx_counters.fx25_blocked++;
	if (decision.allowed == 0) {
		runtime->prepared_counters.bridge_blocked++;
		return KN_AX25_RUNTIME_ERR_DISABLED;
	}

	runtime->prepared_tx_counters.blocked++;
	runtime->prepared_counters.bridge_blocked++;
	return KN_AX25_RUNTIME_ERR_DISABLED;
}

void
kn_ax25_runtime_free(struct kn_ax25_runtime *runtime)
{
	if (runtime == NULL)
		return;

	kn_ax25_connection_table_free(&runtime->table);
	kn_ax25_scheduler_reset(&runtime->scheduler);
	kn_ax25_live_scheduler_reset(&runtime->live_scheduler);
	kn_ax25_prepared_queue_free(&runtime->prepared_queue);
	memset(runtime, 0, sizeof(*runtime));
}

const struct kn_ax25_connection_record *
kn_ax25_runtime_get_connection(const struct kn_ax25_runtime *runtime,
	size_t index)
{
	if (runtime == NULL || index >= runtime->table.count)
		return NULL;
	if (runtime->table.records[index].in_use == 0)
		return NULL;

	return &runtime->table.records[index];
}

void
kn_ax25_runtime_init(struct kn_ax25_runtime *runtime)
{
	if (runtime == NULL)
		return;

	memset(runtime, 0, sizeof(*runtime));
	runtime->diagnostics_enabled = 1;
	runtime->live.live_rx_retain_frame_plans = 1;
	runtime->max_connections = KN_AX25_CONNECTION_TABLE_DEFAULT_MAX;
	kn_ax25_params_default(&runtime->params);
	runtime->params.allow_connected_mode = 0;
	kn_ax25_connection_table_init(&runtime->table);
	kn_ax25_scheduler_init(&runtime->scheduler);
	kn_ax25_live_scheduler_init(&runtime->live_scheduler);
	kn_ax25_prepared_queue_init(&runtime->prepared_queue);
	kn_ax25_prepared_policy_default(&runtime->prepared_policy);
	kn_ax25_prepared_tx_policy_default(&runtime->prepared_tx_policy);
	runtime->table.max_connections = runtime->max_connections;
	runtime_apply_table_params(runtime);
}

enum kn_ax25_runtime_error
kn_ax25_runtime_inject_event(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_connection_event_record *event,
	struct kn_ax25_connection_table_result *result)
{
	enum kn_ax25_connection_table_error table_rc;

	if (runtime == NULL || event == NULL || result == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;

	kn_ax25_connection_table_result_clear(result);
	if (runtime->enabled == 0 || (runtime->connected_mode_enabled == 0 &&
	    runtime->live.live_rx_feed == 0)) {
		runtime->counters.events_rejected++;
		return KN_AX25_RUNTIME_ERR_DISABLED;
	}
	if (event->kind == KN_AX25_CONNECTION_EVENT_NONE) {
		runtime->counters.events_rejected++;
		runtime->counters.unsupported_frame_events++;
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;
	}

	table_rc = kn_ax25_connection_table_process(&runtime->table, event,
	    result);
	if (table_rc != KN_AX25_CONNECTION_TABLE_OK) {
		runtime->counters.events_rejected++;
		if (table_rc == KN_AX25_CONNECTION_TABLE_ERR_NOT_FOUND)
			runtime->counters.unsupported_frame_events++;
		return KN_AX25_RUNTIME_ERR_TABLE;
	}

	runtime->counters.events_accepted++;
	if (result->created != 0)
		runtime->counters.connections_created++;
	runtime->counters.frame_plans_generated += result->plans.count;
	(void)kn_ax25_runtime_prepare_plans(runtime,
	    (uint32_t)(result->record_index + 1U), event->key.port_name,
	    event->timestamp, &result->plans);
	if (result->state_status != KN_AX25_STATE_OK)
		runtime->counters.protocol_errors++;

	return KN_AX25_RUNTIME_OK;
}

enum kn_ax25_runtime_error
kn_ax25_runtime_prepare_plans(struct kn_ax25_runtime *runtime,
	uint32_t connection_id, const char *port_name, uint64_t now_ms,
	const struct kn_ax25_frame_plan_list *plans)
{
	struct kn_ax25_prepared_frame prepared;
	uint64_t id;
	size_t i;
	enum kn_ax25_prepared_queue_error queue_rc;

	if (runtime == NULL || port_name == NULL || plans == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	if (connection_id == 0)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;
	if (runtime->prepared_policy.enabled == 0)
		return KN_AX25_RUNTIME_OK;
	if (kn_ax25_prepared_policy_validate(&runtime->prepared_policy) !=
	    KN_AX25_PREPARED_POLICY_OK)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	for (i = 0; i < plans->count; i++) {
		runtime->prepared_counters.frames_attempted++;
		if (kn_ax25_prepared_frame_from_plan(&prepared,
		    &plans->plans[i], connection_id, port_name, now_ms,
		    runtime->prepared_policy.build_raw) !=
		    KN_AX25_PREPARED_FRAME_OK) {
			runtime->prepared_counters.build_failures++;
			continue;
		}
		if (prepared.status ==
		    KN_AX25_PREPARED_FRAME_STATUS_BUILD_FAILED)
			runtime->prepared_counters.build_failures++;
		queue_rc = kn_ax25_prepared_queue_push(
		    &runtime->prepared_queue, &prepared, &id);
		if (queue_rc == KN_AX25_PREPARED_QUEUE_ERR_FULL) {
			runtime->prepared_counters.queue_full++;
			continue;
		}
		if (queue_rc != KN_AX25_PREPARED_QUEUE_OK) {
			runtime->prepared_counters.build_failures++;
			continue;
		}
		runtime->prepared_counters.frames_stored++;
	}

	return KN_AX25_RUNTIME_OK;
}

size_t
kn_ax25_runtime_connection_count(const struct kn_ax25_runtime *runtime)
{
	if (runtime == NULL)
		return 0;

	return kn_ax25_connection_table_count(&runtime->table);
}

void
kn_ax25_runtime_reset(struct kn_ax25_runtime *runtime)
{
	struct kn_ax25_params params;
	struct kn_ax25_live_options live;
	struct kn_ax25_scheduler_policy scheduler_policy;
	struct kn_ax25_prepared_policy prepared_policy;
	struct kn_ax25_prepared_tx_policy prepared_tx_policy;
	size_t max_connections;
	uint8_t enabled;
	uint8_t connected_mode_enabled;
	uint8_t diagnostics_enabled;

	if (runtime == NULL)
		return;

	params = runtime->params;
	live = runtime->live;
	scheduler_policy = runtime->live_scheduler.policy;
	prepared_policy = runtime->prepared_policy;
	prepared_tx_policy = runtime->prepared_tx_policy;
	max_connections = runtime->max_connections;
	enabled = runtime->enabled;
	connected_mode_enabled = runtime->connected_mode_enabled;
	diagnostics_enabled = runtime->diagnostics_enabled;
	memset(&runtime->counters, 0, sizeof(runtime->counters));
	memset(&runtime->live_counters, 0, sizeof(runtime->live_counters));
	memset(&runtime->prepared_counters, 0,
	    sizeof(runtime->prepared_counters));
	memset(&runtime->prepared_tx_counters, 0,
	    sizeof(runtime->prepared_tx_counters));
	kn_ax25_connection_table_reset(&runtime->table);
	kn_ax25_scheduler_reset(&runtime->scheduler);
	kn_ax25_live_scheduler_reset(&runtime->live_scheduler);
	kn_ax25_prepared_queue_reset(&runtime->prepared_queue);
	runtime->params = params;
	runtime->live = live;
	runtime->live_scheduler.policy = scheduler_policy;
	runtime->prepared_policy = prepared_policy;
	runtime->prepared_tx_policy = prepared_tx_policy;
	runtime->prepared_queue.max_frames = prepared_policy.max_frames;
	runtime->max_connections = max_connections;
	runtime->enabled = enabled;
	runtime->connected_mode_enabled = connected_mode_enabled;
	runtime->diagnostics_enabled = diagnostics_enabled;
	runtime->table.max_connections = max_connections;
	runtime_apply_table_params(runtime);
}

enum kn_ax25_runtime_error
kn_ax25_runtime_set_enabled(struct kn_ax25_runtime *runtime, uint8_t enabled,
	uint8_t connected_mode_enabled)
{
	if (runtime == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	if (enabled > 1 || connected_mode_enabled > 1)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	runtime->enabled = enabled;
	runtime->connected_mode_enabled = connected_mode_enabled;
	runtime->params.allow_connected_mode = connected_mode_enabled;
	runtime_apply_table_params(runtime);
	return KN_AX25_RUNTIME_OK;
}

enum kn_ax25_runtime_error
kn_ax25_runtime_set_live_options(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_live_options *options)
{
	if (runtime == NULL || options == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	if (options->live_rx_feed > 1 ||
	    options->live_rx_create_connections > 1 ||
	    options->live_rx_retain_frame_plans > 1)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;
	if (options->live_rx_feed != 0 && runtime->enabled == 0)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;
	if (options->live_rx_create_connections != 0 &&
	    options->live_rx_feed == 0)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	runtime->live = *options;
	runtime_apply_table_params(runtime);
	return KN_AX25_RUNTIME_OK;
}

enum kn_ax25_runtime_error
kn_ax25_runtime_set_scheduler_policy(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_scheduler_policy *policy)
{
	if (runtime == NULL || policy == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	if (policy->enabled != 0 && runtime->enabled == 0)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;
	if (kn_ax25_live_scheduler_set_policy(&runtime->live_scheduler,
	    policy) != KN_AX25_LIVE_SCHEDULER_OK)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	return KN_AX25_RUNTIME_OK;
}

enum kn_ax25_runtime_error
kn_ax25_runtime_set_prepared_policy(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_prepared_policy *policy)
{
	if (runtime == NULL || policy == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	if (kn_ax25_prepared_policy_validate(policy) !=
	    KN_AX25_PREPARED_POLICY_OK)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;
	if (kn_ax25_prepared_queue_set_max(&runtime->prepared_queue,
	    policy->max_frames) != KN_AX25_PREPARED_QUEUE_OK)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	runtime->prepared_policy = *policy;
	return KN_AX25_RUNTIME_OK;
}

enum kn_ax25_runtime_error
kn_ax25_runtime_set_prepared_tx_policy(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_prepared_tx_policy *policy)
{
	if (runtime == NULL || policy == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	if (kn_ax25_prepared_tx_policy_validate(policy) !=
	    KN_AX25_PREPARED_TX_POLICY_OK)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	runtime->prepared_tx_policy = *policy;
	return KN_AX25_RUNTIME_OK;
}

enum kn_ax25_runtime_error
kn_ax25_runtime_set_params(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_params *params, size_t max_connections)
{
	if (runtime == NULL || params == NULL)
		return KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT;
	if (max_connections == 0 ||
	    max_connections > KN_AX25_CONNECTION_TABLE_MAX)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;
	if (kn_ax25_params_validate(params) != KN_AX25_PARAMS_OK)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	runtime->params = *params;
	runtime->connected_mode_enabled = params->allow_connected_mode;
	runtime->max_connections = max_connections;
	runtime->table.max_connections = max_connections;
	runtime_apply_table_params(runtime);

	return KN_AX25_RUNTIME_OK;
}
