/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_scheduler.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_scheduler.h"

static enum kn_ax25_scheduler_error apply_action(
	struct kn_ax25_scheduler *, uint32_t, const struct kn_ax25_params *,
	const struct kn_ax25_action *, uint64_t);
static enum kn_ax25_scheduler_error get_connection(
	struct kn_ax25_scheduler *, uint32_t, uint8_t, uint8_t,
	struct kn_ax25_scheduler_connection **);
static enum kn_ax25_scheduler_error map_queue_error(
	enum kn_ax25_timer_queue_error);
static uint32_t timer_duration(const struct kn_ax25_params *,
	enum kn_ax25_timer_kind);

static enum kn_ax25_scheduler_error
apply_action(struct kn_ax25_scheduler *scheduler, uint32_t connection_id,
	const struct kn_ax25_params *params, const struct kn_ax25_action *action,
	uint64_t now_ms)
{
	struct kn_ax25_scheduler_connection *entry;
	enum kn_ax25_timer_queue_error queue_rc;

	if (action == NULL)
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;

	switch (action->intent) {
	case KN_AX25_ACTION_START_T1:
		queue_rc = kn_ax25_timer_queue_start(&scheduler->queue,
		    connection_id, KN_AX25_TIMER_T1, now_ms,
		    timer_duration(params, KN_AX25_TIMER_T1));
		if (queue_rc != KN_AX25_TIMER_QUEUE_OK)
			return map_queue_error(queue_rc);
		scheduler->counters.timers_started++;
		return KN_AX25_SCHEDULER_OK;
	case KN_AX25_ACTION_STOP_T1:
		queue_rc = kn_ax25_timer_queue_stop(&scheduler->queue,
		    connection_id, KN_AX25_TIMER_T1);
		if (queue_rc == KN_AX25_TIMER_QUEUE_OK)
			scheduler->counters.timers_stopped++;
		if (queue_rc == KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND)
			return KN_AX25_SCHEDULER_OK;
		return map_queue_error(queue_rc);
	case KN_AX25_ACTION_START_T3:
		queue_rc = kn_ax25_timer_queue_start(&scheduler->queue,
		    connection_id, KN_AX25_TIMER_T3, now_ms,
		    timer_duration(params, KN_AX25_TIMER_T3));
		if (queue_rc != KN_AX25_TIMER_QUEUE_OK)
			return map_queue_error(queue_rc);
		scheduler->counters.timers_started++;
		return KN_AX25_SCHEDULER_OK;
	case KN_AX25_ACTION_STOP_T3:
		queue_rc = kn_ax25_timer_queue_stop(&scheduler->queue,
		    connection_id, KN_AX25_TIMER_T3);
		if (queue_rc == KN_AX25_TIMER_QUEUE_OK)
			scheduler->counters.timers_stopped++;
		if (queue_rc == KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND)
			return KN_AX25_SCHEDULER_OK;
		return map_queue_error(queue_rc);
	case KN_AX25_ACTION_RESET_RETRY_COUNT:
		if (get_connection(scheduler, connection_id,
		    params->n2_retry_count, 1, &entry) !=
		    KN_AX25_SCHEDULER_OK)
			return KN_AX25_SCHEDULER_ERR_FULL;
		kn_ax25_retry_reset(&entry->retry);
		return KN_AX25_SCHEDULER_OK;
	case KN_AX25_ACTION_INCREMENT_RETRY_COUNT:
		if (get_connection(scheduler, connection_id,
		    params->n2_retry_count, 1, &entry) !=
		    KN_AX25_SCHEDULER_OK)
			return KN_AX25_SCHEDULER_ERR_FULL;
		if (kn_ax25_retry_increment(&entry->retry) !=
		    KN_AX25_RETRY_OK) {
			scheduler->counters.retries_exhausted++;
			return KN_AX25_SCHEDULER_ERR_INVALID_VALUE;
		}
		scheduler->counters.retries_incremented++;
		if (kn_ax25_retry_exhausted(&entry->retry) != 0)
			scheduler->counters.retries_exhausted++;
		return KN_AX25_SCHEDULER_OK;
	case KN_AX25_ACTION_NONE:
	case KN_AX25_ACTION_SEND_SABM:
	case KN_AX25_ACTION_SEND_SABME:
	case KN_AX25_ACTION_SEND_UA:
	case KN_AX25_ACTION_SEND_DM:
	case KN_AX25_ACTION_SEND_DISC:
	case KN_AX25_ACTION_SEND_RR:
	case KN_AX25_ACTION_SEND_RNR:
	case KN_AX25_ACTION_SEND_REJ:
	case KN_AX25_ACTION_SEND_FRMR:
	case KN_AX25_ACTION_DELIVER_I_PAYLOAD:
	case KN_AX25_ACTION_ENTER_CONNECTED:
	case KN_AX25_ACTION_ENTER_DISCONNECTED:
	case KN_AX25_ACTION_PROTOCOL_ERROR:
	case KN_AX25_ACTION_RETRANSMIT_NEEDED:
		return KN_AX25_SCHEDULER_OK;
	}

	return KN_AX25_SCHEDULER_ERR_INVALID_VALUE;
}

static enum kn_ax25_scheduler_error
get_connection(struct kn_ax25_scheduler *scheduler, uint32_t connection_id,
	uint8_t max_retries, uint8_t create,
	struct kn_ax25_scheduler_connection **entry)
{
	size_t i;

	if (scheduler == NULL || entry == NULL)
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;

	for (i = 0; i < scheduler->connection_count; i++) {
		if (scheduler->connections[i].in_use != 0 &&
		    scheduler->connections[i].connection_id == connection_id) {
			*entry = &scheduler->connections[i];
			return KN_AX25_SCHEDULER_OK;
		}
	}
	if (create == 0)
		return KN_AX25_SCHEDULER_ERR_NOT_FOUND;
	if (scheduler->connection_count >= KN_AX25_SCHEDULER_CONNECTION_MAX)
		return KN_AX25_SCHEDULER_ERR_FULL;
	if (kn_ax25_retry_validate_max(max_retries) != KN_AX25_RETRY_OK)
		return KN_AX25_SCHEDULER_ERR_INVALID_VALUE;

	i = scheduler->connection_count;
	memset(&scheduler->connections[i], 0, sizeof(scheduler->connections[i]));
	scheduler->connections[i].in_use = 1;
	scheduler->connections[i].connection_id = connection_id;
	(void)kn_ax25_retry_init(&scheduler->connections[i].retry,
	    max_retries);
	scheduler->connection_count++;
	*entry = &scheduler->connections[i];
	return KN_AX25_SCHEDULER_OK;
}

static enum kn_ax25_scheduler_error
map_queue_error(enum kn_ax25_timer_queue_error error)
{
	switch (error) {
	case KN_AX25_TIMER_QUEUE_OK:
		return KN_AX25_SCHEDULER_OK;
	case KN_AX25_TIMER_QUEUE_ERR_FULL:
		return KN_AX25_SCHEDULER_ERR_FULL;
	case KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND:
		return KN_AX25_SCHEDULER_ERR_NOT_FOUND;
	case KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT:
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;
	case KN_AX25_TIMER_QUEUE_ERR_INVALID_VALUE:
	case KN_AX25_TIMER_QUEUE_ERR_BUFFER:
		break;
	}

	return KN_AX25_SCHEDULER_ERR_TIMER_QUEUE;
}

static uint32_t
timer_duration(const struct kn_ax25_params *params,
	enum kn_ax25_timer_kind kind)
{
	switch (kind) {
	case KN_AX25_TIMER_T1:
		return params->t1_ms;
	case KN_AX25_TIMER_T2:
		return params->t2_ms;
	case KN_AX25_TIMER_T3:
		return params->t3_ms;
	case KN_AX25_TIMER_NONE:
	case KN_AX25_TIMER_IDLE_SESSION:
	case KN_AX25_TIMER_BUSY_CONDITION:
		break;
	}

	return 0;
}

enum kn_ax25_scheduler_error
kn_ax25_scheduler_apply_actions(struct kn_ax25_scheduler *scheduler,
	uint32_t connection_id, const struct kn_ax25_params *params,
	const struct kn_ax25_action_list *actions, uint64_t now_ms)
{
	size_t i;
	enum kn_ax25_scheduler_error rc;

	if (scheduler == NULL || params == NULL || actions == NULL)
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;
	if (kn_ax25_params_validate(params) != KN_AX25_PARAMS_OK)
		return KN_AX25_SCHEDULER_ERR_INVALID_VALUE;

	for (i = 0; i < actions->count; i++) {
		rc = apply_action(scheduler, connection_id, params,
		    &actions->actions[i], now_ms);
		if (rc != KN_AX25_SCHEDULER_OK)
			return rc;
	}

	return KN_AX25_SCHEDULER_OK;
}

enum kn_ax25_scheduler_error
kn_ax25_scheduler_format(const struct kn_ax25_scheduler *scheduler,
	char *buf, size_t bufsiz)
{
	int needed;

	if (scheduler == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "connections=%llu timers=%llu running=%llu started=%llu "
	    "stopped=%llu expired=%llu retries=%llu exhausted=%llu "
	    "tx_writes=%llu",
	    (unsigned long long)scheduler->connection_count,
	    (unsigned long long)scheduler->queue.count,
	    (unsigned long long)kn_ax25_timer_queue_count_running(
	    &scheduler->queue),
	    (unsigned long long)scheduler->counters.timers_started,
	    (unsigned long long)scheduler->counters.timers_stopped,
	    (unsigned long long)scheduler->counters.timers_expired,
	    (unsigned long long)scheduler->counters.retries_incremented,
	    (unsigned long long)scheduler->counters.retries_exhausted,
	    (unsigned long long)scheduler->counters.tx_queue_writes_attempted);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_SCHEDULER_ERR_BUFFER;

	return KN_AX25_SCHEDULER_OK;
}

void
kn_ax25_scheduler_init(struct kn_ax25_scheduler *scheduler)
{
	if (scheduler == NULL)
		return;

	memset(scheduler, 0, sizeof(*scheduler));
	kn_ax25_timer_queue_init(&scheduler->queue);
}

enum kn_ax25_scheduler_error
kn_ax25_scheduler_next_wakeup(const struct kn_ax25_scheduler *scheduler,
	uint64_t *expires_at_ms)
{
	enum kn_ax25_timer_queue_error rc;

	if (scheduler == NULL || expires_at_ms == NULL)
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;

	rc = kn_ax25_timer_queue_next_expiry(&scheduler->queue,
	    expires_at_ms);
	return map_queue_error(rc);
}

enum kn_ax25_scheduler_error
kn_ax25_scheduler_poll_expired(struct kn_ax25_scheduler *scheduler,
	uint64_t now_ms, struct kn_ax25_timer_expiry *expired,
	size_t expired_len, size_t *expired_count)
{
	enum kn_ax25_timer_queue_error rc;

	if (scheduler == NULL)
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;

	rc = kn_ax25_timer_queue_collect_expired(&scheduler->queue, now_ms,
	    expired, expired_len, expired_count);
	if (rc != KN_AX25_TIMER_QUEUE_OK)
		return map_queue_error(rc);
	scheduler->counters.timers_expired += *expired_count;
	return KN_AX25_SCHEDULER_OK;
}

enum kn_ax25_scheduler_error
kn_ax25_scheduler_process_expiry(struct kn_ax25_scheduler *scheduler,
	uint32_t connection_id, enum kn_ax25_timer_kind kind,
	struct kn_ax25_connection *connection, uint64_t now_ms,
	struct kn_ax25_scheduler_expiry_result *result)
{
	struct kn_ax25_state_input input;
	struct kn_ax25_state_result state_result;
	enum kn_ax25_connection_event event;
	enum kn_ax25_state_error state_rc;

	if (scheduler == NULL || connection == NULL || result == NULL)
		return KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT;

	kn_ax25_scheduler_result_clear(result);
	event = kn_ax25_timer_kind_event(kind);
	result->connection_id = connection_id;
	result->kind = kind;
	result->event = event;
	if (event == KN_AX25_CONNECTION_EVENT_NONE)
		return KN_AX25_SCHEDULER_ERR_INVALID_VALUE;

	kn_ax25_state_input_clear(&input);
	input.event = event;
	kn_ax25_state_result_clear(&state_result);
	state_rc = kn_ax25_state_step(connection, &input, &state_result);
	result->state_status = state_rc;
	result->actions = state_result.actions;
	(void)kn_ax25_scheduler_apply_actions(scheduler, connection_id,
	    &connection->params, &state_result.actions, now_ms);
	if (state_rc != KN_AX25_STATE_OK &&
	    state_rc != KN_AX25_STATE_ERR_INVALID_EVENT)
		return KN_AX25_SCHEDULER_ERR_STATE;

	return KN_AX25_SCHEDULER_OK;
}

void
kn_ax25_scheduler_reset(struct kn_ax25_scheduler *scheduler)
{
	if (scheduler == NULL)
		return;

	memset(scheduler->connections, 0, sizeof(scheduler->connections));
	scheduler->connection_count = 0;
	memset(&scheduler->counters, 0, sizeof(scheduler->counters));
	kn_ax25_timer_queue_reset(&scheduler->queue);
}

void
kn_ax25_scheduler_result_clear(struct kn_ax25_scheduler_expiry_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	kn_ax25_action_list_clear(&result->actions);
}
