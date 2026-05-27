/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_connection_table.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_connection_table.h"

static uint8_t event_can_create(enum kn_ax25_connection_event);
static void record_init(struct kn_ax25_connection_record *,
	const struct kn_ax25_connection_key *, const struct kn_ax25_params *,
	uint64_t);
static void record_update_counters(struct kn_ax25_connection_record *,
	enum kn_ax25_connection_event, enum kn_ax25_state_error);
static enum kn_ax25_connection_table_error result_from_mapper(
	enum kn_ax25_action_mapper_error);
static void setup_mapper_context(const struct kn_ax25_connection_record *,
	const struct kn_ax25_connection_event_record *,
	struct kn_ax25_action_mapper_context *);

static uint8_t
event_can_create(enum kn_ax25_connection_event event)
{
	return event == KN_AX25_CONNECTION_EVENT_RX_SABM ||
	    event == KN_AX25_CONNECTION_EVENT_RX_SABME ||
	    event == KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST;
}

static void
record_init(struct kn_ax25_connection_record *record,
	const struct kn_ax25_connection_key *key,
	const struct kn_ax25_params *params, uint64_t now)
{
	memset(record, 0, sizeof(*record));
	record->in_use = 1;
	record->key = *key;
	record->params = *params;
	record->created = now;
	record->last_event = now;
	kn_ax25_connection_init(&record->connection, params);
	record->connection.local = key->local;
	record->connection.remote = key->remote;
}

static void
record_update_counters(struct kn_ax25_connection_record *record,
	enum kn_ax25_connection_event event, enum kn_ax25_state_error status)
{
	switch (event) {
	case KN_AX25_CONNECTION_EVENT_RX_SABM:
	case KN_AX25_CONNECTION_EVENT_RX_SABME:
		record->counters.rx_sabm_sabme++;
		break;
	case KN_AX25_CONNECTION_EVENT_RX_UA:
		record->counters.rx_ua++;
		break;
	case KN_AX25_CONNECTION_EVENT_RX_DISC:
		record->counters.rx_disc++;
		break;
	case KN_AX25_CONNECTION_EVENT_RX_DM:
		record->counters.rx_dm++;
		break;
	case KN_AX25_CONNECTION_EVENT_RX_I:
		record->counters.rx_i++;
		break;
	case KN_AX25_CONNECTION_EVENT_RX_RR:
	case KN_AX25_CONNECTION_EVENT_RX_RNR:
	case KN_AX25_CONNECTION_EVENT_RX_REJ:
		record->counters.rx_s_frames++;
		break;
	default:
		break;
	}
	if (status != KN_AX25_STATE_OK)
		record->counters.protocol_errors++;
}

static enum kn_ax25_connection_table_error
result_from_mapper(enum kn_ax25_action_mapper_error error)
{
	if (error == KN_AX25_ACTION_MAPPER_OK)
		return KN_AX25_CONNECTION_TABLE_OK;

	return KN_AX25_CONNECTION_TABLE_ERR_MAPPER;
}

static void
setup_mapper_context(const struct kn_ax25_connection_record *record,
	const struct kn_ax25_connection_event_record *event,
	struct kn_ax25_action_mapper_context *context)
{
	size_t i;

	kn_ax25_action_mapper_context_clear(context);
	context->local = record->key.local;
	context->remote = record->key.remote;
	context->digipeater_count = record->key.digipeater_count;
	for (i = 0; i < record->key.digipeater_count; i++)
		context->digipeaters[i] = record->key.digipeaters[i];
	context->receive_state = record->connection.receive_state;
	context->send_state = record->connection.send_state;
	context->modulo_mode = record->connection.params.modulo_mode;
	context->poll_final = event->poll_final;
}

void
kn_ax25_connection_table_free(struct kn_ax25_connection_table *table)
{
	kn_ax25_connection_table_reset(table);
}

enum kn_ax25_connection_table_error
kn_ax25_connection_table_find(const struct kn_ax25_connection_table *table,
	const struct kn_ax25_connection_key *key, size_t *index)
{
	size_t i;

	if (table == NULL || key == NULL || index == NULL)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_validate(key) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_VALUE;

	for (i = 0; i < table->count; i++) {
		if (table->records[i].in_use != 0 &&
		    kn_ax25_connection_key_equal(&table->records[i].key,
		    key) != 0) {
			*index = i;
			return KN_AX25_CONNECTION_TABLE_OK;
		}
	}

	return KN_AX25_CONNECTION_TABLE_ERR_NOT_FOUND;
}

struct kn_ax25_connection_record *
kn_ax25_connection_table_get(struct kn_ax25_connection_table *table,
	size_t index)
{
	if (table == NULL || index >= table->count)
		return NULL;
	if (table->records[index].in_use == 0)
		return NULL;

	return &table->records[index];
}

enum kn_ax25_connection_table_error
kn_ax25_connection_table_get_or_create(struct kn_ax25_connection_table *table,
	const struct kn_ax25_connection_key *key, uint64_t now,
	size_t *index, uint8_t *created)
{
	enum kn_ax25_connection_table_error rc;

	if (table == NULL || key == NULL || index == NULL)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_validate(key) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_VALUE;

	rc = kn_ax25_connection_table_find(table, key, index);
	if (rc == KN_AX25_CONNECTION_TABLE_OK) {
		if (created != NULL)
			*created = 0;
		return KN_AX25_CONNECTION_TABLE_OK;
	}
	if (rc != KN_AX25_CONNECTION_TABLE_ERR_NOT_FOUND)
		return rc;
	if (table->count >= table->max_connections ||
	    table->count >= KN_AX25_CONNECTION_TABLE_MAX)
		return KN_AX25_CONNECTION_TABLE_ERR_FULL;

	*index = table->count;
	record_init(&table->records[*index], key, &table->params, now);
	table->count++;
	if (created != NULL)
		*created = 1;
	return KN_AX25_CONNECTION_TABLE_OK;
}

void
kn_ax25_connection_table_init(struct kn_ax25_connection_table *table)
{
	if (table == NULL)
		return;

	memset(table, 0, sizeof(*table));
	kn_ax25_params_default(&table->params);
	table->max_connections = KN_AX25_CONNECTION_TABLE_DEFAULT_MAX;
}

enum kn_ax25_connection_table_error
kn_ax25_connection_table_list(const struct kn_ax25_connection_table *table,
	const struct kn_ax25_connection_record **records, size_t records_len,
	size_t *count)
{
	size_t i;

	if (table == NULL || count == NULL)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT;
	if (records == NULL && records_len > 0)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT;

	*count = table->count;
	if (records == NULL)
		return KN_AX25_CONNECTION_TABLE_OK;
	if (records_len < table->count)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_VALUE;
	for (i = 0; i < table->count; i++)
		records[i] = &table->records[i];

	return KN_AX25_CONNECTION_TABLE_OK;
}

enum kn_ax25_connection_table_error
kn_ax25_connection_table_process(struct kn_ax25_connection_table *table,
	const struct kn_ax25_connection_event_record *event,
	struct kn_ax25_connection_table_result *result)
{
	struct kn_ax25_connection_record *record;
	struct kn_ax25_state_input input;
	struct kn_ax25_state_result state_result;
	struct kn_ax25_action_mapper_context mapper;
	size_t index;
	uint8_t created;
	enum kn_ax25_connection_table_error rc;
	enum kn_ax25_action_mapper_error mapper_rc;

	if (table == NULL || event == NULL || result == NULL)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_validate(&event->key) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_VALUE;

	kn_ax25_connection_table_result_clear(result);
	rc = kn_ax25_connection_table_find(table, &event->key, &index);
	if (rc == KN_AX25_CONNECTION_TABLE_ERR_NOT_FOUND) {
		if (event_can_create(event->kind) == 0)
			return KN_AX25_CONNECTION_TABLE_ERR_NOT_FOUND;
		rc = kn_ax25_connection_table_get_or_create(table,
		    &event->key, event->timestamp, &index, &created);
		if (rc != KN_AX25_CONNECTION_TABLE_OK)
			return rc;
		result->created = created;
	} else if (rc != KN_AX25_CONNECTION_TABLE_OK) {
		return rc;
	}

	record = kn_ax25_connection_table_get(table, index);
	if (record == NULL)
		return KN_AX25_CONNECTION_TABLE_ERR_NOT_FOUND;
	if (kn_ax25_connection_event_to_state_input(event, &input) !=
	    KN_AX25_CONNECTION_EVENT_OK)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_VALUE;

	result->record_index = index;
	record->last_event = event->timestamp;
	record->last_event_kind = event->kind;
	record->last_control = event->control;

	kn_ax25_state_result_clear(&state_result);
	(void)kn_ax25_state_step(&record->connection, &input,
	    &state_result);
	record->last_state_status = state_result.status;
	record->last_actions = state_result.actions;
	result->state_status = state_result.status;
	result->actions = state_result.actions;

	setup_mapper_context(record, event, &mapper);
	kn_ax25_frame_plan_list_clear(&record->last_plans);
	mapper_rc = kn_ax25_action_mapper_map_list(&mapper,
	    &state_result.actions, &record->last_plans);
	result->mapper_status = mapper_rc;
	if (mapper_rc != KN_AX25_ACTION_MAPPER_OK)
		return result_from_mapper(mapper_rc);
	result->plans = record->last_plans;

	record_update_counters(record, event->kind, state_result.status);
	return KN_AX25_CONNECTION_TABLE_OK;
}

enum kn_ax25_connection_table_error
kn_ax25_connection_table_remove(struct kn_ax25_connection_table *table,
	const struct kn_ax25_connection_key *key)
{
	size_t index;
	size_t i;
	enum kn_ax25_connection_table_error rc;

	if (table == NULL || key == NULL)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT;
	rc = kn_ax25_connection_table_find(table, key, &index);
	if (rc != KN_AX25_CONNECTION_TABLE_OK)
		return rc;

	for (i = index; i + 1 < table->count; i++)
		table->records[i] = table->records[i + 1];
	memset(&table->records[table->count - 1], 0,
	    sizeof(table->records[table->count - 1]));
	table->count--;
	return KN_AX25_CONNECTION_TABLE_OK;
}

void
kn_ax25_connection_table_reset(struct kn_ax25_connection_table *table)
{
	if (table == NULL)
		return;

	memset(table->records, 0, sizeof(table->records));
	table->count = 0;
}

enum kn_ax25_connection_table_error
kn_ax25_connection_table_set_params(struct kn_ax25_connection_table *table,
	const struct kn_ax25_params *params)
{
	if (table == NULL || params == NULL)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT;
	if (kn_ax25_params_validate(params) != KN_AX25_PARAMS_OK)
		return KN_AX25_CONNECTION_TABLE_ERR_INVALID_VALUE;

	table->params = *params;
	if (table->max_connections == 0 ||
	    table->max_connections > KN_AX25_CONNECTION_TABLE_MAX)
		table->max_connections = KN_AX25_CONNECTION_TABLE_DEFAULT_MAX;
	return KN_AX25_CONNECTION_TABLE_OK;
}

void
kn_ax25_connection_table_result_clear(
	struct kn_ax25_connection_table_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	kn_ax25_action_list_clear(&result->actions);
	kn_ax25_frame_plan_list_clear(&result->plans);
}

size_t
kn_ax25_connection_table_count(const struct kn_ax25_connection_table *table)
{
	if (table == NULL)
		return 0;

	return table->count;
}
