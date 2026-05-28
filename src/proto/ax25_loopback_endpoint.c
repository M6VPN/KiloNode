/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_endpoint.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_frame_builder.h"
#include "kilonode/ax25_loopback_endpoint.h"
#include "kilonode/buffer.h"

static enum kn_ax25_loopback_endpoint_error build_i_frame(
	struct kn_ax25_loopback_endpoint *, const uint8_t *, size_t,
	uint8_t *, size_t, size_t *);
static enum kn_ax25_loopback_endpoint_error prepare_plans(
	struct kn_ax25_loopback_endpoint *,
	const struct kn_ax25_frame_plan_list *, uint32_t);
static enum kn_ax25_loopback_endpoint_error process_event(
	struct kn_ax25_loopback_endpoint *,
	const struct kn_ax25_connection_event_record *);
static void set_error(struct kn_ax25_loopback_endpoint *, const char *);

static enum kn_ax25_loopback_endpoint_error
build_i_frame(struct kn_ax25_loopback_endpoint *endpoint,
	const uint8_t *payload, size_t payload_len, uint8_t *out,
	size_t out_len, size_t *written)
{
	struct kn_ax25_connection_record *record;
	struct kn_ax25_addr addr;
	struct kn_buffer frame;
	enum kn_ax25_error ax25_rc;
	uint8_t control;

	if (endpoint == NULL || payload == NULL || out == NULL ||
	    written == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (payload_len == 0 || payload_len > KN_AX25_LOOPBACK_I_PAYLOAD_MAX)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE;
	if (endpoint->table.count == 0)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_STATE;
	record = &endpoint->table.records[0];
	if (record->connection.state != KN_AX25_CONNECTION_CONNECTED)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_STATE;
	if (kn_buffer_init(&frame, 0) != 0)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_BUFFER;

	memset(&addr, 0, sizeof(addr));
	addr.callsign = endpoint->peer;
	ax25_rc = kn_ax25_address_encode(&addr, 0, &frame);
	if (ax25_rc != KN_AX25_OK)
		goto fail;
	memset(&addr, 0, sizeof(addr));
	addr.callsign = endpoint->local;
	ax25_rc = kn_ax25_address_encode(&addr, 1, &frame);
	if (ax25_rc != KN_AX25_OK)
		goto fail;

	control = (uint8_t)(((record->connection.send_state & 0x07U) << 1) |
	    ((record->connection.receive_state & 0x07U) << 5));
	if (kn_buffer_append_byte(&frame, control) != 0 ||
	    kn_buffer_append_byte(&frame, KN_AX25_PID_NO_LAYER_3) != 0 ||
	    kn_buffer_append(&frame, payload, payload_len) != 0)
		goto fail;
	if (frame.len > out_len)
		goto fail;

	memcpy(out, frame.data, frame.len);
	*written = frame.len;
	record->connection.send_state =
	    (uint8_t)((record->connection.send_state + 1U) & 0x07U);
	kn_buffer_free(&frame);
	return KN_AX25_LOOPBACK_ENDPOINT_OK;

fail:
	kn_buffer_free(&frame);
	return KN_AX25_LOOPBACK_ENDPOINT_ERR_BUFFER;
}

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
	const struct kn_ax25_connection_event_record *event)
{
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;
	size_t i;
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
	for (i = 0; i < result.actions.count; i++) {
		if (result.actions.actions[i].intent ==
		    KN_AX25_ACTION_DELIVER_I_PAYLOAD)
			endpoint->delivered_payloads++;
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
	    "prepared=%llu delivered=%llu tx_writes=%llu dispatch=%llu "
	    "fx25=%llu",
	    endpoint->name, local, peer,
	    kn_ax25_connection_state_name(endpoint->last_state),
	    (unsigned long long)endpoint->inbound_frames,
	    (unsigned long long)endpoint->outbound_prepared_frames,
	    (unsigned long long)endpoint->delivered_payloads,
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
	return process_event(endpoint, &event);
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
	return process_event(endpoint, &event);
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_process_frame(
	struct kn_ax25_loopback_endpoint *endpoint, const uint8_t *data,
	size_t data_len)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_connection_event_record event;
	enum kn_ax25_connection_event_error event_rc;

	if (endpoint == NULL || data == NULL || data_len == 0)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;
	if (kn_ax25_frame_decode(data, data_len, &frame) != KN_AX25_OK) {
		set_error(endpoint, "decode");
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_DECODE;
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
	return process_event(endpoint, &event);
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
	endpoint->last_state = KN_AX25_CONNECTION_DISCONNECTED;
}

enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_send_i(struct kn_ax25_loopback_endpoint *endpoint,
	const uint8_t *payload, size_t payload_len, uint8_t *out,
	size_t out_len, size_t *written)
{
	if (endpoint == NULL)
		return KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT;

	return build_i_frame(endpoint, payload, payload_len, out, out_len,
	    written);
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
