/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_endpoint.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_control.h"
#include "kilonode/ax25_frame_builder.h"
#include "kilonode/ax25_i_frame.h"
#include "kilonode/ax25_loopback_endpoint.h"
#include "kilonode/ax25_loopback_payload.h"

static enum kn_ax25_loopback_endpoint_error prepare_plans(
	struct kn_ax25_loopback_endpoint *,
	const struct kn_ax25_frame_plan_list *, uint32_t);
static enum kn_ax25_loopback_endpoint_error process_event(
	struct kn_ax25_loopback_endpoint *,
	const struct kn_ax25_connection_event_record *,
	const struct kn_ax25_i_frame_decoded *);
static void set_error(struct kn_ax25_loopback_endpoint *, const char *);
static void sync_window_counters(struct kn_ax25_loopback_endpoint *);

static enum kn_ax25_loopback_endpoint_error
prepare_plans(struct kn_ax25_loopback_endpoint *endpoint,
	const struct kn_ax25_frame_plan_list *plans, uint32_t connection_id)
{
	struct kn_ax25_prepared_frame prepared;
	size_t i;
	uint64_t id;

	if (endpoint == NULL || plans == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;

	for (i = 0; i < plans->count; i++) {
		if (kn_ax25_prepared_frame_from_plan(&prepared,
		    &plans->plans[i], connection_id, endpoint->port_name,
		    endpoint->now_ms, 1) != KN_AX25_PREPARED_FRAME_OK)
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_PREPARED;
		if (kn_ax25_prepared_queue_push(&endpoint->prepared,
		    &prepared, &id) != KN_AX25_PREPARED_QUEUE_OK)
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_PREPARED;
		endpoint->outbound_prepared_frames++;
	}

	return KN_AX25_LOOPBACK_ENDPOINT_OK;
}

static enum kn_ax25_loopback_endpoint_error
process_event(struct kn_ax25_loopback_endpoint *endpoint,
	const struct kn_ax25_connection_event_record *event,
	const struct kn_ax25_i_frame_decoded *i_frame)
{
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;
	size_t i;
	uint8_t delivered;
	uint32_t connection_id;
	enum kn_ax25_connection_table_error table_rc;

	if (endpoint == NULL || event == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;

	kn_ax25_connection_table_result_clear(&result);
	table_rc = kn_ax25_connection_table_process(&endpoint->table, event,
	    &result);
	if (table_rc != KN_AX25_CONNECTION_TABLE_OK) {
		set_error(endpoint, "connection-table");
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_TABLE;
	}

	connection_id = (uint32_t)(result.record_index + 1U);
	record = kn_ax25_connection_table_get(&endpoint->table,
	    result.record_index);
	if (record != NULL)
		endpoint->last_state = record->connection.state;
	delivered = 0;
	for (i = 0; i < result.actions.count; i++) {
		if (result.actions.actions[i].intent ==
		    KN_AX25_ACTION_DELIVER_I_PAYLOAD) {
			endpoint->delivered_payloads++;
			delivered = 1;
		}
		if (result.actions.actions[i].intent == KN_AX25_ACTION_SEND_RR)
			endpoint->rr_frames_sent++;
		if (result.actions.actions[i].intent == KN_AX25_ACTION_SEND_REJ)
			endpoint->rejected_payloads++;
	}
	if (i_frame != NULL) {
		if (delivered != 0) {
			if (kn_ax25_loopback_payload_record(endpoint, i_frame,
			    1, "accepted") != KN_AX25_LOOPBACK_PAYLOAD_OK)
				return KN_AX25_LOOPBACK_ENDPOINT_ERR_BUFFER;
		} else {
			if (kn_ax25_loopback_payload_record(endpoint, i_frame,
			    0, "sequence") != KN_AX25_LOOPBACK_PAYLOAD_OK)
				return KN_AX25_LOOPBACK_ENDPOINT_ERR_BUFFER;
		}
	}
	if (record != NULL &&
	    kn_ax25_scheduler_apply_actions(&endpoint->scheduler,
	    connection_id, &record->params, &result.actions,
	    endpoint->now_ms) != KN_AX25_SCHEDULER_OK) {
		set_error(endpoint, "scheduler");
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_SCHEDULER;
	}
	if (prepare_plans(endpoint, &result.plans, connection_id) !=
	    KN_AX25_LOOPBACK_ENDPOINT_OK) {
		set_error(endpoint, "prepared");
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_PREPARED;
	}

	return KN_AX25_LOOPBACK_ENDPOINT_OK;
}

static void
set_error(struct kn_ax25_loopback_endpoint *endpoint, const char *text)
{
	int needed;

	if (endpoint == NULL)
		return;
	if (text == NULL)
		text = "";
	needed = snprintf(endpoint->last_error, sizeof(endpoint->last_error),
	    "%s", text);
	if (needed < 0 || (size_t)needed >= sizeof(endpoint->last_error))
		endpoint->last_error[sizeof(endpoint->last_error) - 1] = '\0';
}

static void
sync_window_counters(struct kn_ax25_loopback_endpoint *endpoint)
{
	if (endpoint == NULL)
		return;

	endpoint->outstanding_frames =
	    kn_ax25_loopback_window_in_flight(&endpoint->window);
	endpoint->outstanding_max_seen = endpoint->window.max_in_flight_seen;
	endpoint->outstanding_acked = endpoint->window.acked;
	endpoint->outstanding_rejected = endpoint->window.rejected;
	endpoint->window_blocked = endpoint->window.blocked;
}

void
kn_ax25_loopback_endpoint_free(struct kn_ax25_loopback_endpoint *endpoint)
{
	kn_ax25_loopback_endpoint_reset(endpoint);
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_format(
	const struct kn_ax25_loopback_endpoint *endpoint, char *buf,
	size_t bufsiz)
{
	char local[KN_CALLSIGN_MAX + 4];
	char peer[KN_CALLSIGN_MAX + 4];
	int needed;

	if (endpoint == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&endpoint->local, local, sizeof(local)) != 0 ||
	    kn_callsign_format(&endpoint->peer, peer, sizeof(peer)) != 0)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "endpoint=%s local=%s peer=%s state=%s inbound=%llu "
	    "prepared=%llu delivered=%llu rejected=%llu reassembled=%llu "
	    "segments_rx=%llu i_rx=%llu rr_rx=%llu outstanding=%llu "
	    "outstanding_max=%llu acked=%llu window_blocked=%llu "
	    "tx_writes=%llu dispatch=%llu fx25=%llu",
	    endpoint->name, local, peer,
	    kn_ax25_connection_state_name(endpoint->last_state),
	    (unsigned long long)endpoint->inbound_frames,
	    (unsigned long long)endpoint->outbound_prepared_frames,
	    (unsigned long long)endpoint->delivered_payloads,
	    (unsigned long long)endpoint->rejected_payloads,
	    (unsigned long long)endpoint->reassembled_payloads,
	    (unsigned long long)endpoint->segments_received,
	    (unsigned long long)endpoint->i_frames_received,
	    (unsigned long long)endpoint->rr_frames_received,
	    (unsigned long long)endpoint->outstanding_frames,
	    (unsigned long long)endpoint->outstanding_max_seen,
	    (unsigned long long)endpoint->outstanding_acked,
	    (unsigned long long)endpoint->window_blocked,
	    (unsigned long long)endpoint->tx_queue_writes,
	    (unsigned long long)endpoint->dispatch_calls,
	    (unsigned long long)endpoint->fx25_frames);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_BUFFER;

	return KN_AX25_LOOPBACK_ENDPOINT_OK;
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_init(struct kn_ax25_loopback_endpoint *endpoint,
	const char *name, const char *local, const char *peer,
	const char *port, const struct kn_ax25_params *params)
{
	struct kn_ax25_connection_key key;
	struct kn_ax25_params defaults;
	int needed;

	if (endpoint == NULL || name == NULL || local == NULL ||
	    peer == NULL || port == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	kn_ax25_loopback_endpoint_reset(endpoint);
	needed = snprintf(endpoint->name, sizeof(endpoint->name), "%s",
	    name);
	if (needed < 0 || (size_t)needed >= sizeof(endpoint->name))
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	if (params == NULL) {
		kn_ax25_params_default(&defaults);
		defaults.allow_connected_mode = 1;
		params = &defaults;
	}
	if (kn_ax25_params_validate(params) != KN_AX25_PARAMS_OK)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	if (kn_ax25_connection_key_from_callsigns(&key, port, local, peer,
	    NULL, 0) != KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;

	endpoint->local = key.local;
	endpoint->peer = key.remote;
	needed = snprintf(endpoint->port_name, sizeof(endpoint->port_name),
	    "%s", port);
	if (needed < 0 || (size_t)needed >= sizeof(endpoint->port_name))
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	endpoint->params = *params;
	endpoint->params.allow_connected_mode = 1;
	kn_ax25_connection_table_init(&endpoint->table);
	(void)kn_ax25_connection_table_set_params(&endpoint->table,
	    &endpoint->params);
	kn_ax25_scheduler_init(&endpoint->scheduler);
	kn_ax25_prepared_queue_init(&endpoint->prepared);
	kn_ax25_payload_delivery_queue_init(&endpoint->deliveries);
	kn_ax25_reassembly_queue_init(&endpoint->reassemblies);
	kn_ax25_loopback_window_init(&endpoint->window);
	endpoint->last_state = KN_AX25_CONNECTION_DISCONNECTED;
	set_error(endpoint, "ok");
	return KN_AX25_LOOPBACK_ENDPOINT_OK;
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_local_connect(
	struct kn_ax25_loopback_endpoint *endpoint)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_key key;

	if (endpoint == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_from_callsigns(&key, endpoint->port_name,
	    endpoint->local.call, endpoint->peer.call, NULL, 0) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	key.local = endpoint->local;
	key.remote = endpoint->peer;
	if (kn_ax25_connection_event_local_connect(&event, endpoint->now_ms,
	    &key) != KN_AX25_CONNECTION_EVENT_OK)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	return process_event(endpoint, &event, NULL);
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_local_disconnect(
	struct kn_ax25_loopback_endpoint *endpoint)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_key key;

	if (endpoint == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_from_callsigns(&key, endpoint->port_name,
	    endpoint->local.call, endpoint->peer.call, NULL, 0) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	key.local = endpoint->local;
	key.remote = endpoint->peer;
	if (kn_ax25_connection_event_local_disconnect(&event,
	    endpoint->now_ms, &key) != KN_AX25_CONNECTION_EVENT_OK)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	return process_event(endpoint, &event, NULL);
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_process_frame(
	struct kn_ax25_loopback_endpoint *endpoint, const uint8_t *data,
	size_t data_len)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_i_frame_decoded i_frame;
	struct kn_ax25_connection_event_record event;
	enum kn_ax25_connection_event_error event_rc;
	struct kn_ax25_control_info control;
	size_t acked;
	size_t rejected;
	enum kn_ax25_loopback_endpoint_error rc;

	if (endpoint == NULL || data == NULL || data_len == 0)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (kn_ax25_frame_decode(data, data_len, &frame) != KN_AX25_OK) {
		set_error(endpoint, "decode");
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_DECODE;
	}
	kn_ax25_control_decode(frame.control, &control);
	kn_ax25_i_frame_decoded_clear(&i_frame);
	if (control.class == KN_AX25_CONTROL_CLASS_I) {
		if (kn_ax25_i_frame_decode_raw(data, data_len, &i_frame) !=
		    KN_AX25_I_FRAME_OK) {
			set_error(endpoint, "i-frame");
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_DECODE;
		}
		endpoint->i_frames_received++;
	} else if (control.class == KN_AX25_CONTROL_CLASS_S &&
	    control.s_subtype == KN_AX25_S_SUBTYPE_RR) {
		endpoint->rr_frames_received++;
	}
	event_rc = kn_ax25_connection_event_from_frame_pair(&event,
	    endpoint->now_ms, endpoint->port_name, &endpoint->local,
	    &frame);
	if (event_rc != KN_AX25_CONNECTION_EVENT_OK) {
		set_error(endpoint, "event");
		return event_rc == KN_AX25_CONNECTION_EVENT_ERR_IGNORED ?
		    KN_AX25_LOOPBACK_ENDPOINT_ERR_UNSUPPORTED :
		    KN_AX25_LOOPBACK_ENDPOINT_ERR_DECODE;
	}
	endpoint->inbound_frames++;
	rc = process_event(endpoint, &event,
	    control.class == KN_AX25_CONTROL_CLASS_I ? &i_frame : NULL);
	if (rc != KN_AX25_LOOPBACK_ENDPOINT_OK)
		return rc;
	if (control.class == KN_AX25_CONTROL_CLASS_S &&
	    control.s_subtype == KN_AX25_S_SUBTYPE_RR) {
		if (kn_ax25_loopback_window_ack_rr(&endpoint->window,
		    control.nr, &acked) != KN_AX25_LOOPBACK_WINDOW_OK)
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_STATE;
	}
	if (control.class == KN_AX25_CONTROL_CLASS_S &&
	    control.s_subtype == KN_AX25_S_SUBTYPE_REJ) {
		if (kn_ax25_loopback_window_reject(&endpoint->window,
		    control.nr, &rejected) != KN_AX25_LOOPBACK_WINDOW_OK)
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_STATE;
	}
	sync_window_counters(endpoint);
	return KN_AX25_LOOPBACK_ENDPOINT_OK;
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_process_timers(
	struct kn_ax25_loopback_endpoint *endpoint, size_t max_expired,
	size_t *processed)
{
	struct kn_ax25_timer_expiry
	    expiries[KN_AX25_LOOPBACK_ENDPOINT_STEP_MAX];
	struct kn_ax25_scheduler_expiry_result expiry_result;
	struct kn_ax25_connection_record *record;
	struct kn_ax25_action_mapper_context mapper;
	struct kn_ax25_frame_plan_list plans;
	size_t count;
	size_t i;
	uint32_t connection_id;

	if (endpoint == NULL || processed == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (max_expired > KN_AX25_LOOPBACK_ENDPOINT_STEP_MAX)
		max_expired = KN_AX25_LOOPBACK_ENDPOINT_STEP_MAX;
	*processed = 0;
	if (kn_ax25_scheduler_poll_expired(&endpoint->scheduler,
	    endpoint->now_ms, expiries, max_expired, &count) !=
	    KN_AX25_SCHEDULER_OK)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_SCHEDULER;

	for (i = 0; i < count; i++) {
		connection_id = expiries[i].connection_id;
		if (connection_id == 0 ||
		    connection_id > endpoint->table.count)
			continue;
		record = &endpoint->table.records[connection_id - 1U];
		if (kn_ax25_scheduler_process_expiry(&endpoint->scheduler,
		    connection_id, expiries[i].kind, &record->connection,
		    endpoint->now_ms, &expiry_result) !=
		    KN_AX25_SCHEDULER_OK)
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_SCHEDULER;
		kn_ax25_action_mapper_context_clear(&mapper);
		mapper.local = record->key.local;
		mapper.remote = record->key.remote;
		mapper.receive_state = record->connection.receive_state;
		mapper.send_state = record->connection.send_state;
		mapper.modulo_mode = record->params.modulo_mode;
		mapper.poll_final = 1;
		kn_ax25_frame_plan_list_clear(&plans);
		if (kn_ax25_action_mapper_map_list(&mapper,
		    &expiry_result.actions, &plans) !=
		    KN_AX25_ACTION_MAPPER_OK)
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_STATE;
		if (prepare_plans(endpoint, &plans, connection_id) !=
		    KN_AX25_LOOPBACK_ENDPOINT_OK)
			return KN_AX25_LOOPBACK_ENDPOINT_ERR_PREPARED;
		endpoint->last_state = record->connection.state;
		(*processed)++;
	}

	return KN_AX25_LOOPBACK_ENDPOINT_OK;
}

void
kn_ax25_loopback_endpoint_reset(struct kn_ax25_loopback_endpoint *endpoint)
{
	if (endpoint == NULL)
		return;

	memset(endpoint, 0, sizeof(*endpoint));
	kn_ax25_params_default(&endpoint->params);
	endpoint->params.allow_connected_mode = 1;
	kn_ax25_connection_table_init(&endpoint->table);
	(void)kn_ax25_connection_table_set_params(&endpoint->table,
	    &endpoint->params);
	kn_ax25_scheduler_init(&endpoint->scheduler);
	kn_ax25_prepared_queue_init(&endpoint->prepared);
	kn_ax25_payload_delivery_queue_init(&endpoint->deliveries);
	kn_ax25_reassembly_queue_init(&endpoint->reassemblies);
	kn_ax25_loopback_window_init(&endpoint->window);
	endpoint->last_state = KN_AX25_CONNECTION_DISCONNECTED;
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_send_i(struct kn_ax25_loopback_endpoint *endpoint,
	const uint8_t *payload, size_t payload_len, uint8_t ns_override,
	uint8_t use_ns_override, uint8_t *out, size_t out_len,
	size_t *written)
{
	if (endpoint == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;

	return kn_ax25_loopback_payload_send_i(endpoint, payload, payload_len,
	    ns_override, use_ns_override, out, out_len, written) ==
	    KN_AX25_LOOPBACK_PAYLOAD_OK ? KN_AX25_LOOPBACK_ENDPOINT_OK :
	    KN_AX25_LOOPBACK_ENDPOINT_ERR_BUFFER;
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_state(
	const struct kn_ax25_loopback_endpoint *endpoint,
	enum kn_ax25_connection_state *state)
{
	if (endpoint == NULL || state == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (endpoint->table.count == 0) {
		*state = KN_AX25_CONNECTION_DISCONNECTED;
		return KN_AX25_LOOPBACK_ENDPOINT_OK;
	}
	*state = endpoint->table.records[0].connection.state;
	return KN_AX25_LOOPBACK_ENDPOINT_OK;
}
