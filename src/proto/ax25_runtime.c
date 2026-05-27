/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_runtime.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_runtime.h"

void
kn_ax25_runtime_free(struct kn_ax25_runtime *runtime)
{
	if (runtime == NULL)
		return;

	kn_ax25_connection_table_free(&runtime->table);
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
	runtime->max_connections = KN_AX25_CONNECTION_TABLE_DEFAULT_MAX;
	kn_ax25_params_default(&runtime->params);
	runtime->params.allow_connected_mode = 0;
	kn_ax25_connection_table_init(&runtime->table);
	runtime->table.max_connections = runtime->max_connections;
	(void)kn_ax25_connection_table_set_params(&runtime->table,
	    &runtime->params);
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
	if (runtime->enabled == 0 || runtime->connected_mode_enabled == 0) {
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
	if (result->state_status != KN_AX25_STATE_OK)
		runtime->counters.protocol_errors++;

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
	size_t max_connections;
	uint8_t enabled;
	uint8_t connected_mode_enabled;
	uint8_t diagnostics_enabled;

	if (runtime == NULL)
		return;

	params = runtime->params;
	max_connections = runtime->max_connections;
	enabled = runtime->enabled;
	connected_mode_enabled = runtime->connected_mode_enabled;
	diagnostics_enabled = runtime->diagnostics_enabled;
	memset(&runtime->counters, 0, sizeof(runtime->counters));
	kn_ax25_connection_table_reset(&runtime->table);
	runtime->params = params;
	runtime->max_connections = max_connections;
	runtime->enabled = enabled;
	runtime->connected_mode_enabled = connected_mode_enabled;
	runtime->diagnostics_enabled = diagnostics_enabled;
	runtime->table.max_connections = max_connections;
	(void)kn_ax25_connection_table_set_params(&runtime->table, &params);
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
	(void)kn_ax25_connection_table_set_params(&runtime->table,
	    &runtime->params);
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
	if (kn_ax25_connection_table_set_params(&runtime->table,
	    params) != KN_AX25_CONNECTION_TABLE_OK)
		return KN_AX25_RUNTIME_ERR_INVALID_VALUE;

	return KN_AX25_RUNTIME_OK;
}
