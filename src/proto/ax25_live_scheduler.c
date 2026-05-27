/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_live_scheduler.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_action_mapper.h"
#include "kilonode/ax25_live_scheduler.h"
#include "kilonode/ax25_runtime.h"

static uint64_t count_blocked_expired(const struct kn_ax25_runtime *,
	uint64_t);
static void mapper_context_from_record(
	const struct kn_ax25_connection_record *,
	struct kn_ax25_action_mapper_context *);
static void record_generated_plans(struct kn_ax25_runtime *,
	struct kn_ax25_connection_record *,
	uint32_t, uint64_t, const struct kn_ax25_action_list *);
static uint64_t tx_action_count(const struct kn_ax25_action_list *);

static uint64_t
count_blocked_expired(const struct kn_ax25_runtime *runtime, uint64_t now_ms)
{
	const struct kn_ax25_timer *timer;
	size_t i;
	uint64_t count;

	if (runtime == NULL)
		return 0;

	count = 0;
	for (i = 0; i < runtime->scheduler.queue.count; i++) {
		timer = &runtime->scheduler.queue.timers[i];
		if (kn_ax25_timer_is_expired(timer, now_ms) != 0)
			count++;
	}

	return count;
}

static void
mapper_context_from_record(const struct kn_ax25_connection_record *record,
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
}

static void
record_generated_plans(struct kn_ax25_runtime *runtime,
	struct kn_ax25_connection_record *record, uint32_t connection_id,
	uint64_t now_ms,
	const struct kn_ax25_action_list *actions)
{
	struct kn_ax25_action_mapper_context context;

	if (runtime == NULL || record == NULL || actions == NULL)
		return;

	record->last_actions = *actions;
	runtime->live_scheduler.tx_actions_blocked += tx_action_count(actions);
	kn_ax25_frame_plan_list_clear(&record->last_plans);
	mapper_context_from_record(record, &context);
	if (kn_ax25_action_mapper_map_list(&context, actions,
	    &record->last_plans) != KN_AX25_ACTION_MAPPER_OK) {
		runtime->live_scheduler.last_error =
		    KN_AX25_LIVE_SCHEDULER_ERR_MAPPER;
		return;
	}
	runtime->live_scheduler.generated_frame_plans +=
	    record->last_plans.count;
	(void)kn_ax25_runtime_prepare_plans(runtime, connection_id,
	    record->key.port_name, now_ms, &record->last_plans);
}

static uint64_t
tx_action_count(const struct kn_ax25_action_list *actions)
{
	size_t i;
	uint64_t count;

	if (actions == NULL)
		return 0;

	count = 0;
	for (i = 0; i < actions->count; i++) {
		switch (actions->actions[i].intent) {
		case KN_AX25_ACTION_SEND_SABM:
		case KN_AX25_ACTION_SEND_SABME:
		case KN_AX25_ACTION_SEND_UA:
		case KN_AX25_ACTION_SEND_DM:
		case KN_AX25_ACTION_SEND_DISC:
		case KN_AX25_ACTION_SEND_RR:
		case KN_AX25_ACTION_SEND_RNR:
		case KN_AX25_ACTION_SEND_REJ:
		case KN_AX25_ACTION_SEND_FRMR:
		case KN_AX25_ACTION_RETRANSMIT_NEEDED:
			count++;
			break;
		default:
			break;
		}
	}

	return count;
}

void
kn_ax25_live_scheduler_init(struct kn_ax25_live_scheduler *live)
{
	if (live == NULL)
		return;

	memset(live, 0, sizeof(*live));
	kn_ax25_scheduler_policy_default(&live->policy);
}

enum kn_ax25_live_scheduler_error
kn_ax25_live_scheduler_poll(struct kn_ax25_runtime *runtime, uint64_t now_ms)
{
	struct kn_ax25_timer_expiry expired[KN_AX25_LIVE_SCHEDULER_EXPIRED_MAX];
	struct kn_ax25_scheduler_expiry_result result;
	struct kn_ax25_connection_record *record;
	size_t count;
	size_t i;
	size_t index;
	enum kn_ax25_scheduler_error scheduler_rc;
	enum kn_ax25_live_scheduler_error rc;

	if (runtime == NULL)
		return KN_AX25_LIVE_SCHEDULER_ERR_INVALID_ARGUMENT;
	if (kn_ax25_scheduler_policy_validate(&runtime->live_scheduler.policy) !=
	    KN_AX25_SCHEDULER_POLICY_OK) {
		runtime->live_scheduler.last_error =
		    KN_AX25_LIVE_SCHEDULER_ERR_INVALID_VALUE;
		return KN_AX25_LIVE_SCHEDULER_ERR_INVALID_VALUE;
	}

	if (runtime->live_scheduler.policy.enabled == 0)
		return KN_AX25_LIVE_SCHEDULER_OK;

	runtime->live_scheduler.cycles_run++;
	runtime->live_scheduler.last_poll_ms = now_ms;
	if (kn_ax25_scheduler_next_wakeup(&runtime->scheduler,
	    &runtime->live_scheduler.next_wakeup_ms) !=
	    KN_AX25_SCHEDULER_OK)
		runtime->live_scheduler.next_wakeup_ms = 0;
	if (runtime->live_scheduler.policy.process_expired == 0) {
		runtime->live_scheduler.expired_blocked_by_policy +=
		    count_blocked_expired(runtime, now_ms);
		return KN_AX25_LIVE_SCHEDULER_OK;
	}

	count = 0;
	scheduler_rc = kn_ax25_scheduler_poll_expired(&runtime->scheduler,
	    now_ms, expired, runtime->live_scheduler.policy.max_expired_per_cycle,
	    &count);
	if (scheduler_rc != KN_AX25_SCHEDULER_OK) {
		runtime->live_scheduler.last_error =
		    KN_AX25_LIVE_SCHEDULER_ERR_SCHEDULER;
		return KN_AX25_LIVE_SCHEDULER_ERR_SCHEDULER;
	}

	for (i = 0; i < count; i++) {
		runtime->live_scheduler.expired_processed++;
		if (expired[i].planned != 0 ||
		    expired[i].connection_id == 0)
			continue;
		index = (size_t)expired[i].connection_id - 1U;
		record = kn_ax25_connection_table_get(&runtime->table, index);
		if (record == NULL)
			continue;
		kn_ax25_scheduler_result_clear(&result);
		scheduler_rc = kn_ax25_scheduler_process_expiry(
		    &runtime->scheduler, expired[i].connection_id,
		    expired[i].kind, &record->connection, now_ms, &result);
		if (scheduler_rc != KN_AX25_SCHEDULER_OK &&
		    scheduler_rc != KN_AX25_SCHEDULER_ERR_STATE) {
			runtime->live_scheduler.last_error =
			    KN_AX25_LIVE_SCHEDULER_ERR_SCHEDULER;
			return KN_AX25_LIVE_SCHEDULER_ERR_SCHEDULER;
		}
		record->last_event = now_ms;
		record->last_event_kind = result.event;
		record->last_state_status = result.state_status;
		record_generated_plans(runtime, record,
		    expired[i].connection_id, now_ms, &result.actions);
	}

	rc = KN_AX25_LIVE_SCHEDULER_OK;
	if (kn_ax25_scheduler_next_wakeup(&runtime->scheduler,
	    &runtime->live_scheduler.next_wakeup_ms) != KN_AX25_SCHEDULER_OK)
		runtime->live_scheduler.next_wakeup_ms = 0;
	runtime->live_scheduler.tx_writes_attempted =
	    runtime->scheduler.counters.tx_queue_writes_attempted;
	runtime->live_scheduler.last_error = rc;
	return rc;
}

void
kn_ax25_live_scheduler_reset(struct kn_ax25_live_scheduler *live)
{
	struct kn_ax25_scheduler_policy policy;

	if (live == NULL)
		return;

	policy = live->policy;
	memset(live, 0, sizeof(*live));
	live->policy = policy;
}

enum kn_ax25_live_scheduler_error
kn_ax25_live_scheduler_set_policy(struct kn_ax25_live_scheduler *live,
	const struct kn_ax25_scheduler_policy *policy)
{
	if (live == NULL || policy == NULL)
		return KN_AX25_LIVE_SCHEDULER_ERR_INVALID_ARGUMENT;
	if (kn_ax25_scheduler_policy_validate(policy) !=
	    KN_AX25_SCHEDULER_POLICY_OK)
		return KN_AX25_LIVE_SCHEDULER_ERR_INVALID_VALUE;

	live->policy = *policy;
	live->last_error = KN_AX25_LIVE_SCHEDULER_OK;
	return KN_AX25_LIVE_SCHEDULER_OK;
}
