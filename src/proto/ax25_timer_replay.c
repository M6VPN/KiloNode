/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_timer_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_action_mapper.h"
#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_timer_replay.h"

#define TIMER_REPLAY_CONNECTION_ID 1U

struct replay_context {
	struct kn_ax25_runtime runtime;
	struct kn_ax25_connection_key key;
	uint64_t now_ms;
	char last_error[KN_AX25_TIMER_REPLAY_TEXT_MAX];
	struct kn_ax25_timer_replay_result *result;
};

static void add_mismatch(struct replay_context *, size_t, const char *);
static uint8_t action_seen(const struct kn_ax25_action_list *,
	enum kn_ax25_action_intent);
static uint64_t counter_value(const struct replay_context *,
	enum kn_ax25_timer_replay_counter);
static enum kn_ax25_timer_replay_error execute_command(
	struct replay_context *,
	const struct kn_ax25_timer_replay_command *);
static enum kn_ax25_timer_replay_error execute_event(
	struct replay_context *,
	const struct kn_ax25_timer_replay_command *);
static enum kn_ax25_timer_replay_error execute_expect(
	struct replay_context *,
	const struct kn_ax25_timer_replay_command *);
static enum kn_ax25_timer_replay_error execute_process_expired(
	struct replay_context *,
	const struct kn_ax25_timer_replay_command *);
static enum kn_ax25_timer_replay_error map_actions_for_record(
	struct replay_context *, struct kn_ax25_connection_record *,
	const struct kn_ax25_action_list *);
static uint8_t plan_seen(const struct kn_ax25_frame_plan_list *,
	enum kn_ax25_frame_plan_type);
static uint8_t prepared_seen(const struct kn_ax25_prepared_queue *,
	const struct kn_ax25_prepared_expect_frame *);
static enum kn_ax25_timer_replay_error replay_init(
	const struct kn_ax25_timer_replay_script *, struct replay_context *,
	struct kn_ax25_timer_replay_result *,
	struct kn_ax25_timer_replay_error_info *);
static void set_last_error(struct replay_context *, const char *);
static enum kn_ax25_timer_replay_error timer_status(
	const struct replay_context *, enum kn_ax25_timer_kind, uint8_t *,
	uint8_t *);
static void update_final(struct replay_context *);

static void
add_mismatch(struct replay_context *ctx, size_t line, const char *detail)
{
	struct kn_ax25_timer_replay_result *result;
	int needed;

	if (ctx == NULL || detail == NULL)
		return;
	result = ctx->result;
	if (result->mismatch_count >= KN_AX25_TIMER_REPLAY_MISMATCH_MAX)
		return;
	result->mismatches[result->mismatch_count].line = line;
	needed = snprintf(result->mismatches[result->mismatch_count].detail,
	    sizeof(result->mismatches[result->mismatch_count].detail), "%s",
	    detail);
	if (needed < 0 || (size_t)needed >=
	    sizeof(result->mismatches[result->mismatch_count].detail))
		result->mismatches[result->mismatch_count].detail[
		    sizeof(result->mismatches[result->mismatch_count].detail) -
		    1] = '\0';
	result->mismatch_count++;
	result->pass = 0;
}

static uint8_t
action_seen(const struct kn_ax25_action_list *actions,
	enum kn_ax25_action_intent action)
{
	size_t i;

	if (actions == NULL)
		return 0;
	for (i = 0; i < actions->count; i++) {
		if (actions->actions[i].intent == action)
			return 1;
	}

	return 0;
}

static uint64_t
counter_value(const struct replay_context *ctx,
	enum kn_ax25_timer_replay_counter counter)
{
	switch (counter) {
	case KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_STARTED:
		return ctx->runtime.scheduler.counters.timers_started;
	case KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_STOPPED:
		return ctx->runtime.scheduler.counters.timers_stopped;
	case KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_EXPIRED:
		return ctx->runtime.scheduler.counters.timers_expired;
	case KN_AX25_TIMER_REPLAY_COUNTER_RETRIES_INCREMENTED:
		return ctx->runtime.scheduler.counters.retries_incremented;
	case KN_AX25_TIMER_REPLAY_COUNTER_RETRIES_EXHAUSTED:
		return ctx->runtime.scheduler.counters.retries_exhausted;
	case KN_AX25_TIMER_REPLAY_COUNTER_FRAME_PLANS:
		return ctx->result->frame_plans_seen;
	case KN_AX25_TIMER_REPLAY_COUNTER_EVENTS_ACCEPTED:
		return ctx->runtime.counters.events_accepted;
	case KN_AX25_TIMER_REPLAY_COUNTER_PROTOCOL_ERRORS:
		return ctx->runtime.counters.protocol_errors;
	}

	return 0;
}

static enum kn_ax25_timer_replay_error
execute_command(struct replay_context *ctx,
	const struct kn_ax25_timer_replay_command *command)
{
	if (ctx == NULL || command == NULL)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT;

	switch (command->type) {
	case KN_AX25_TIMER_REPLAY_CMD_NOW:
		ctx->now_ms = command->value;
		return KN_AX25_TIMER_REPLAY_OK;
	case KN_AX25_TIMER_REPLAY_CMD_ADVANCE:
		if (UINT64_MAX - ctx->now_ms < command->value)
			return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
		ctx->now_ms += command->value;
		return KN_AX25_TIMER_REPLAY_OK;
	case KN_AX25_TIMER_REPLAY_CMD_PARAMS:
		if (kn_ax25_runtime_set_params(&ctx->runtime,
		    &command->params, KN_AX25_CONNECTION_TABLE_DEFAULT_MAX) !=
		    KN_AX25_RUNTIME_OK)
			return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
		return KN_AX25_TIMER_REPLAY_OK;
	case KN_AX25_TIMER_REPLAY_CMD_EVENT:
		return execute_event(ctx, command);
	case KN_AX25_TIMER_REPLAY_CMD_PROCESS_EXPIRED:
		return execute_process_expired(ctx, command);
	case KN_AX25_TIMER_REPLAY_CMD_EXPECT:
		return execute_expect(ctx, command);
	}

	return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
}

static enum kn_ax25_timer_replay_error
execute_event(struct replay_context *ctx,
	const struct kn_ax25_timer_replay_command *command)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result table_result;
	struct kn_ax25_connection_record *record;
	enum kn_ax25_runtime_error runtime_rc;

	kn_ax25_connection_event_clear(&event);
	event.timestamp = ctx->now_ms;
	event.key = ctx->key;
	event.kind = command->event;
	event.ns = command->ns;
	event.nr = command->nr;
	event.poll_final = command->poll_final;
	event.payload_len = command->payload_len;
	if (command->event ==
	    KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST &&
	    kn_ax25_connection_event_local_connect(&event, ctx->now_ms,
	    &ctx->key) != KN_AX25_CONNECTION_EVENT_OK)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	if (command->event ==
	    KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST &&
	    kn_ax25_connection_event_local_disconnect(&event, ctx->now_ms,
	    &ctx->key) != KN_AX25_CONNECTION_EVENT_OK)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;

	kn_ax25_connection_table_result_clear(&table_result);
	runtime_rc = kn_ax25_runtime_inject_event(&ctx->runtime, &event,
	    &table_result);
	if (runtime_rc != KN_AX25_RUNTIME_OK) {
		set_last_error(ctx, "runtime-error");
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	}
	ctx->result->last_actions = table_result.actions;
	ctx->result->last_plans = table_result.plans;
	ctx->result->frame_plans_seen += table_result.plans.count;
	ctx->result->events_accepted = ctx->runtime.counters.events_accepted;
	ctx->result->protocol_errors = ctx->runtime.counters.protocol_errors;
	record = kn_ax25_connection_table_get(&ctx->runtime.table,
	    table_result.record_index);
	if (record == NULL)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	if (kn_ax25_scheduler_apply_actions(&ctx->runtime.scheduler,
	    TIMER_REPLAY_CONNECTION_ID, &record->connection.params,
	    &table_result.actions, ctx->now_ms) != KN_AX25_SCHEDULER_OK)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	if (command->event == KN_AX25_CONNECTION_EVENT_RX_I &&
	    table_result.state_status == KN_AX25_STATE_OK) {
		if (kn_ax25_timer_queue_start(&ctx->runtime.scheduler.queue,
		    TIMER_REPLAY_CONNECTION_ID, KN_AX25_TIMER_T2, ctx->now_ms,
		    ctx->runtime.params.t2_ms) == KN_AX25_TIMER_QUEUE_OK)
			ctx->runtime.scheduler.counters.timers_started++;
	}
	set_last_error(ctx, kn_ax25_state_error_name(table_result.state_status));
	update_final(ctx);
	return KN_AX25_TIMER_REPLAY_OK;
}

static enum kn_ax25_timer_replay_error
execute_expect(struct replay_context *ctx,
	const struct kn_ax25_timer_replay_command *command)
{
	struct kn_ax25_connection_record *record;
	uint8_t running;
	uint8_t expired;
	uint64_t actual;

	record = kn_ax25_connection_table_get(&ctx->runtime.table, 0);
	switch (command->expect.type) {
	case KN_AX25_TIMER_REPLAY_EXPECT_STATE:
		if (ctx->result->final_state != command->expect.state)
			add_mismatch(ctx, command->line, "state");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_RETRY:
		actual = record == NULL ? 0 : record->connection.retry_count;
		if (actual != command->expect.value)
			add_mismatch(ctx, command->line, "retry");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_TIMER_RUNNING:
		if (timer_status(ctx, command->expect.timer_kind, &running,
		    &expired) != KN_AX25_TIMER_REPLAY_OK)
			return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
		if (running != command->expect.bool_value)
			add_mismatch(ctx, command->line, "timer-running");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_TIMER_EXPIRED:
		if (timer_status(ctx, command->expect.timer_kind, &running,
		    &expired) != KN_AX25_TIMER_REPLAY_OK)
			return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
		if (expired != command->expect.bool_value)
			add_mismatch(ctx, command->line, "timer-expired");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_ACTION:
		if (action_seen(&ctx->result->last_actions,
		    command->expect.action) == 0)
			add_mismatch(ctx, command->line, "action");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_PLAN:
		if (plan_seen(&ctx->result->last_plans,
		    command->expect.plan) == 0)
			add_mismatch(ctx, command->line, "plan");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_COUNTER:
		if (counter_value(ctx, command->expect.counter) !=
		    command->expect.value)
			add_mismatch(ctx, command->line, "counter");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_TX_WRITES:
		actual = ctx->runtime.scheduler.counters.tx_queue_writes_attempted;
		actual += ctx->runtime.prepared_counters.tx_queue_writes_attempted;
		if (actual != command->expect.value)
			add_mismatch(ctx, command->line, "tx-writes");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_CONNECTION_COUNT:
		if (kn_ax25_runtime_connection_count(&ctx->runtime) !=
		    command->expect.value)
			add_mismatch(ctx, command->line, "connection-count");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_LAST_ERROR:
		if (strcmp(ctx->last_error, command->expect.last_error) != 0)
			add_mismatch(ctx, command->line, "last-error");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_PREPARED_COUNT:
		if (ctx->runtime.prepared_queue.count != command->expect.value)
			add_mismatch(ctx, command->line, "prepared-count");
		break;
	case KN_AX25_TIMER_REPLAY_EXPECT_PREPARED:
		if (prepared_seen(&ctx->runtime.prepared_queue,
		    &command->expect.prepared) == 0)
			add_mismatch(ctx, command->line, "prepared");
		break;
	}

	return KN_AX25_TIMER_REPLAY_OK;
}

static enum kn_ax25_timer_replay_error
execute_process_expired(struct replay_context *ctx,
	const struct kn_ax25_timer_replay_command *command)
{
	struct kn_ax25_timer_expiry expired[KN_AX25_TIMER_EXPIRED_MAX];
	struct kn_ax25_scheduler_expiry_result expiry_result;
	struct kn_ax25_connection_record *record;
	size_t count;
	size_t i;

	(void)command;
	if (kn_ax25_scheduler_poll_expired(&ctx->runtime.scheduler,
	    ctx->now_ms, expired, KN_AX25_TIMER_EXPIRED_MAX, &count) !=
	    KN_AX25_SCHEDULER_OK)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	for (i = 0; i < count; i++) {
		if (expired[i].planned != 0) {
			set_last_error(ctx, "planned");
			continue;
		}
		record = kn_ax25_connection_table_get(&ctx->runtime.table,
		    expired[i].connection_id - 1U);
		if (record == NULL)
			return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
		kn_ax25_scheduler_result_clear(&expiry_result);
		if (kn_ax25_scheduler_process_expiry(&ctx->runtime.scheduler,
		    expired[i].connection_id, expired[i].kind,
		    &record->connection, ctx->now_ms, &expiry_result) !=
		    KN_AX25_SCHEDULER_OK)
			return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
		ctx->result->last_actions = expiry_result.actions;
		if (map_actions_for_record(ctx, record,
		    &expiry_result.actions) != KN_AX25_TIMER_REPLAY_OK)
			return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
		set_last_error(ctx,
		    kn_ax25_state_error_name(expiry_result.state_status));
	}
	update_final(ctx);
	return KN_AX25_TIMER_REPLAY_OK;
}

static enum kn_ax25_timer_replay_error
map_actions_for_record(struct replay_context *ctx,
	struct kn_ax25_connection_record *record,
	const struct kn_ax25_action_list *actions)
{
	struct kn_ax25_action_mapper_context mapper;

	if (ctx == NULL || record == NULL || actions == NULL)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT;

	kn_ax25_action_mapper_context_clear(&mapper);
	mapper.local = record->key.local;
	mapper.remote = record->key.remote;
	mapper.receive_state = record->connection.receive_state;
	mapper.send_state = record->connection.send_state;
	mapper.modulo_mode = record->connection.params.modulo_mode;
	mapper.poll_final = 1;
	kn_ax25_frame_plan_list_clear(&record->last_plans);
	if (kn_ax25_action_mapper_map_list(&mapper, actions,
	    &record->last_plans) != KN_AX25_ACTION_MAPPER_OK)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	record->last_actions = *actions;
	ctx->result->last_plans = record->last_plans;
	ctx->result->frame_plans_seen += record->last_plans.count;
	(void)kn_ax25_runtime_prepare_plans(&ctx->runtime,
	    TIMER_REPLAY_CONNECTION_ID, record->key.port_name, ctx->now_ms,
	    &record->last_plans);
	return KN_AX25_TIMER_REPLAY_OK;
}

static uint8_t
plan_seen(const struct kn_ax25_frame_plan_list *plans,
	enum kn_ax25_frame_plan_type plan)
{
	size_t i;

	if (plans == NULL)
		return 0;
	for (i = 0; i < plans->count; i++) {
		if (plans->plans[i].type == plan)
			return 1;
	}

	return 0;
}

static uint8_t
prepared_seen(const struct kn_ax25_prepared_queue *queue,
	const struct kn_ax25_prepared_expect_frame *expect)
{
	const struct kn_ax25_prepared_frame *frame;
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];
	size_t i;

	if (queue == NULL || expect == NULL)
		return 0;
	for (i = 0; i < queue->count; i++) {
		frame = &queue->frames[i];
		if (expect->has_kind != 0 && frame->type != expect->kind)
			continue;
		if (expect->has_action != 0 &&
		    frame->action_source != expect->action)
			continue;
		if (expect->has_status != 0 &&
		    frame->status != expect->status)
			continue;
		if (expect->has_local != 0) {
			if (kn_callsign_format(&frame->local, local,
			    sizeof(local)) != 0 ||
			    strcmp(local, expect->local) != 0)
				continue;
		}
		if (expect->has_remote != 0) {
			if (kn_callsign_format(&frame->remote, remote,
			    sizeof(remote)) != 0 ||
			    strcmp(remote, expect->remote) != 0)
				continue;
		}
		if (expect->has_port != 0 &&
		    strcmp(frame->port_name, expect->port) != 0)
			continue;
		if (expect->has_ax25_len != 0 &&
		    frame->raw_len != expect->ax25_len)
			continue;
		return 1;
	}

	return 0;
}

static enum kn_ax25_timer_replay_error
replay_init(const struct kn_ax25_timer_replay_script *script,
	struct replay_context *ctx, struct kn_ax25_timer_replay_result *result,
	struct kn_ax25_timer_replay_error_info *error)
{
	struct kn_ax25_params params;
	int needed;

	if (script == NULL || ctx == NULL || result == NULL)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT;

	memset(ctx, 0, sizeof(*ctx));
	kn_ax25_timer_replay_result_clear(result);
	result->pass = 1;
	needed = snprintf(result->name, sizeof(result->name), "%s",
	    script->name);
	if (needed < 0 || (size_t)needed >= sizeof(result->name))
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	ctx->result = result;
	set_last_error(ctx, "ok");
	if (kn_ax25_connection_key_from_callsigns(&ctx->key, script->port,
	    script->node, script->remote, NULL, 0) !=
	    KN_AX25_CONNECTION_KEY_OK) {
		if (error != NULL) {
			error->error = KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
			error->line = 0;
			(void)snprintf(error->message, sizeof(error->message),
			    "invalid-key");
		}
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	}

	kn_ax25_runtime_init(&ctx->runtime);
	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	if (kn_ax25_runtime_set_params(&ctx->runtime, &params,
	    KN_AX25_CONNECTION_TABLE_DEFAULT_MAX) != KN_AX25_RUNTIME_OK ||
	    kn_ax25_runtime_set_enabled(&ctx->runtime, 1, 1) !=
	    KN_AX25_RUNTIME_OK)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE;
	return KN_AX25_TIMER_REPLAY_OK;
}

static void
set_last_error(struct replay_context *ctx, const char *text)
{
	if (ctx == NULL || text == NULL)
		return;

	(void)snprintf(ctx->last_error, sizeof(ctx->last_error), "%s", text);
}

static enum kn_ax25_timer_replay_error
timer_status(const struct replay_context *ctx, enum kn_ax25_timer_kind kind,
	uint8_t *running, uint8_t *expired)
{
	size_t index;

	if (ctx == NULL || running == NULL || expired == NULL)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT;

	*running = 0;
	*expired = 0;
	if (kn_ax25_timer_queue_find(&ctx->runtime.scheduler.queue,
	    TIMER_REPLAY_CONNECTION_ID, kind, &index) !=
	    KN_AX25_TIMER_QUEUE_OK)
		return KN_AX25_TIMER_REPLAY_OK;
	*running = kn_ax25_timer_is_running(
	    &ctx->runtime.scheduler.queue.timers[index]);
	*expired = kn_ax25_timer_is_expired(
	    &ctx->runtime.scheduler.queue.timers[index], ctx->now_ms);
	return KN_AX25_TIMER_REPLAY_OK;
}

static void
update_final(struct replay_context *ctx)
{
	struct kn_ax25_connection_record *record;
	uint8_t running;
	uint8_t expired;

	if (ctx == NULL)
		return;
	record = kn_ax25_connection_table_get(&ctx->runtime.table, 0);
	ctx->result->connection_count =
	    kn_ax25_runtime_connection_count(&ctx->runtime);
	ctx->result->final_state = record == NULL ?
	    KN_AX25_CONNECTION_DISCONNECTED : record->connection.state;
	ctx->result->retry_count = record == NULL ? 0 :
	    record->connection.retry_count;
	(void)timer_status(ctx, KN_AX25_TIMER_T1, &running, &expired);
	ctx->result->t1_running = running;
	(void)timer_status(ctx, KN_AX25_TIMER_T2, &running, &expired);
	ctx->result->t2_running = running;
	(void)timer_status(ctx, KN_AX25_TIMER_T3, &running, &expired);
	ctx->result->t3_running = running;
	ctx->result->tx_writes_observed =
	    ctx->runtime.scheduler.counters.tx_queue_writes_attempted;
	ctx->result->prepared_count = ctx->runtime.prepared_queue.count;
	ctx->result->prepared_tx_writes_observed =
	    ctx->runtime.prepared_counters.tx_queue_writes_attempted;
	for (running = 0;
	    running < ctx->runtime.prepared_queue.count &&
	    running < KN_AX25_TIMER_REPLAY_PREPARED_MAX; running++)
		ctx->result->prepared[running] =
		    ctx->runtime.prepared_queue.frames[running];
	ctx->result->scheduler_counters = ctx->runtime.scheduler.counters;
}

void
kn_ax25_timer_replay_result_clear(
	struct kn_ax25_timer_replay_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	kn_ax25_action_list_clear(&result->last_actions);
	kn_ax25_frame_plan_list_clear(&result->last_plans);
}

enum kn_ax25_timer_replay_error
kn_ax25_timer_replay_run(const struct kn_ax25_timer_replay_script *script,
	struct kn_ax25_timer_replay_result *result,
	struct kn_ax25_timer_replay_error_info *error)
{
	struct replay_context ctx;
	size_t i;
	enum kn_ax25_timer_replay_error rc;

	kn_ax25_timer_replay_error_clear(error);
	rc = replay_init(script, &ctx, result, error);
	if (rc != KN_AX25_TIMER_REPLAY_OK)
		return rc;
	for (i = 0; i < script->command_count; i++) {
		rc = execute_command(&ctx, &script->commands[i]);
		if (rc != KN_AX25_TIMER_REPLAY_OK) {
			if (error != NULL) {
				error->error = rc;
				error->line = script->commands[i].line;
				(void)snprintf(error->message,
				    sizeof(error->message), "execute");
			}
			return rc;
		}
	}
	update_final(&ctx);
	if (result->mismatch_count > 0)
		return KN_AX25_TIMER_REPLAY_ERR_MISMATCH;
	if (result->tx_writes_observed != 0 ||
	    result->prepared_tx_writes_observed != 0) {
		add_mismatch(&ctx, 0, "tx-writes-nonzero");
		return KN_AX25_TIMER_REPLAY_ERR_MISMATCH;
	}

	return KN_AX25_TIMER_REPLAY_OK;
}

enum kn_ax25_timer_replay_error
kn_ax25_timer_replay_run_file(const char *path,
	struct kn_ax25_timer_replay_result *result,
	struct kn_ax25_timer_replay_error_info *error)
{
	struct kn_ax25_timer_replay_script script;
	enum kn_ax25_timer_replay_error rc;

	rc = kn_ax25_timer_replay_script_parse_file(path, &script, error);
	if (rc != KN_AX25_TIMER_REPLAY_OK)
		return rc;

	return kn_ax25_timer_replay_run(&script, result, error);
}
