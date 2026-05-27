/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_runtime.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/callsign.h"

static int make_key(struct kn_ax25_connection_key *);
static int make_sabm_event(struct kn_ax25_connection_event_record *,
	uint64_t);
static int test_counters_update(void);
static int test_default_runtime_disabled(void);
static int test_disabled_runtime_rejects_injection(void);
static int test_init_with_max_connections(void);
static int test_inject_sabm_creates_connection(void);
static int test_no_tx_queue_writes(void);
static int test_params_stored(void);
static int test_reset_clears_table(void);

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
	if (test_inject_sabm_creates_connection() != 0)
		return 1;
	if (test_counters_update() != 0)
		return 1;
	if (test_reset_clears_table() != 0)
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
