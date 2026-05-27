/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_action_mapper.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_action_mapper.h"

static enum kn_ax25_action_mapper_error action_to_type(
	enum kn_ax25_action_intent, enum kn_ax25_frame_plan_type *);
static enum kn_ax25_action_mapper_error append_plan(
	const struct kn_ax25_action_mapper_context *,
	const struct kn_ax25_action *, enum kn_ax25_frame_plan_type,
	struct kn_ax25_frame_plan_list *);
static uint8_t action_has_sequence(enum kn_ax25_action_intent);
static enum kn_ax25_action_mapper_error context_validate(
	const struct kn_ax25_action_mapper_context *);
static uint8_t intent_emits_frame(enum kn_ax25_action_intent);

static enum kn_ax25_action_mapper_error
action_to_type(enum kn_ax25_action_intent intent,
	enum kn_ax25_frame_plan_type *type)
{
	if (type == NULL)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_ARGUMENT;

	switch (intent) {
	case KN_AX25_ACTION_SEND_SABM:
		*type = KN_AX25_FRAME_PLAN_SABM;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_SABME:
		*type = KN_AX25_FRAME_PLAN_SABME;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_UA:
		*type = KN_AX25_FRAME_PLAN_UA;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_DM:
		*type = KN_AX25_FRAME_PLAN_DM;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_DISC:
		*type = KN_AX25_FRAME_PLAN_DISC;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_RR:
		*type = KN_AX25_FRAME_PLAN_RR;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_RNR:
		*type = KN_AX25_FRAME_PLAN_RNR;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_REJ:
		*type = KN_AX25_FRAME_PLAN_REJ;
		return KN_AX25_ACTION_MAPPER_OK;
	case KN_AX25_ACTION_SEND_FRMR:
		*type = KN_AX25_FRAME_PLAN_FRMR;
		return KN_AX25_ACTION_MAPPER_OK;
	default:
		break;
	}

	return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;
}

static enum kn_ax25_action_mapper_error
append_plan(const struct kn_ax25_action_mapper_context *context,
	const struct kn_ax25_action *action, enum kn_ax25_frame_plan_type type,
	struct kn_ax25_frame_plan_list *plans)
{
	struct kn_ax25_frame_plan plan;
	size_t i;

	memset(&plan, 0, sizeof(plan));
	plan.source = context->local;
	plan.destination = context->remote;
	plan.digipeater_count = context->digipeater_count;
	for (i = 0; i < context->digipeater_count; i++)
		plan.digipeaters[i] = context->digipeaters[i];
	plan.type = type;
	plan.poll_final = context->poll_final;
	plan.nr = action_has_sequence(action->intent) != 0 ?
	    action->sequence : context->receive_state;
	plan.ns = context->send_state;
	plan.action_source = action->intent;
	plan.needs_kiss = 0;
	plan.needs_fx25 = 0;

	if (kn_ax25_frame_plan_list_append(plans, &plan) !=
	    KN_AX25_FRAME_PLAN_OK)
		return KN_AX25_ACTION_MAPPER_ERR_PLAN_OVERFLOW;

	return KN_AX25_ACTION_MAPPER_OK;
}

static uint8_t
action_has_sequence(enum kn_ax25_action_intent intent)
{
	return intent == KN_AX25_ACTION_SEND_RR ||
	    intent == KN_AX25_ACTION_SEND_RNR ||
	    intent == KN_AX25_ACTION_SEND_REJ;
}

static enum kn_ax25_action_mapper_error
context_validate(const struct kn_ax25_action_mapper_context *context)
{
	char text[KN_CALLSIGN_MAX + 4];
	size_t i;

	if (context == NULL)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&context->local, text, sizeof(text)) != 0 ||
	    kn_callsign_format(&context->remote, text, sizeof(text)) != 0)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;
	if (context->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;
	for (i = 0; i < context->digipeater_count; i++) {
		if (kn_callsign_format(&context->digipeaters[i], text,
		    sizeof(text)) != 0)
			return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;
	}
	if (context->receive_state > 7 || context->send_state > 7 ||
	    context->poll_final > 1)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;
	if (context->modulo_mode != KN_AX25_MODULO_8)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;

	return KN_AX25_ACTION_MAPPER_OK;
}

static uint8_t
intent_emits_frame(enum kn_ax25_action_intent intent)
{
	return intent == KN_AX25_ACTION_SEND_SABM ||
	    intent == KN_AX25_ACTION_SEND_SABME ||
	    intent == KN_AX25_ACTION_SEND_UA ||
	    intent == KN_AX25_ACTION_SEND_DM ||
	    intent == KN_AX25_ACTION_SEND_DISC ||
	    intent == KN_AX25_ACTION_SEND_RR ||
	    intent == KN_AX25_ACTION_SEND_RNR ||
	    intent == KN_AX25_ACTION_SEND_REJ ||
	    intent == KN_AX25_ACTION_SEND_FRMR;
}

void
kn_ax25_action_mapper_context_clear(
	struct kn_ax25_action_mapper_context *context)
{
	if (context == NULL)
		return;

	memset(context, 0, sizeof(*context));
	context->modulo_mode = KN_AX25_MODULO_8;
}

enum kn_ax25_action_mapper_error
kn_ax25_action_mapper_context_init(
	struct kn_ax25_action_mapper_context *context, const char *local,
	const char *remote)
{
	if (context == NULL || local == NULL || remote == NULL)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_ARGUMENT;

	kn_ax25_action_mapper_context_clear(context);
	if (kn_callsign_parse(local, &context->local) != 0 ||
	    kn_callsign_parse(remote, &context->remote) != 0)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;

	return KN_AX25_ACTION_MAPPER_OK;
}

enum kn_ax25_action_mapper_error
kn_ax25_action_mapper_map_action(
	const struct kn_ax25_action_mapper_context *context,
	const struct kn_ax25_action *action,
	struct kn_ax25_frame_plan_list *plans)
{
	enum kn_ax25_frame_plan_type type;
	enum kn_ax25_action_mapper_error rc;

	if (action == NULL || plans == NULL)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_ARGUMENT;
	rc = context_validate(context);
	if (rc != KN_AX25_ACTION_MAPPER_OK)
		return rc;

	if (intent_emits_frame(action->intent) == 0)
		return KN_AX25_ACTION_MAPPER_OK;
	if (action_has_sequence(action->intent) != 0 &&
	    action->sequence > 7)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;
	if (action_to_type(action->intent, &type) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE;

	return append_plan(context, action, type, plans);
}

enum kn_ax25_action_mapper_error
kn_ax25_action_mapper_map_list(
	const struct kn_ax25_action_mapper_context *context,
	const struct kn_ax25_action_list *actions,
	struct kn_ax25_frame_plan_list *plans)
{
	size_t i;
	enum kn_ax25_action_mapper_error rc;

	if (actions == NULL || plans == NULL)
		return KN_AX25_ACTION_MAPPER_ERR_INVALID_ARGUMENT;

	for (i = 0; i < actions->count; i++) {
		rc = kn_ax25_action_mapper_map_action(context,
		    &actions->actions[i], plans);
		if (rc != KN_AX25_ACTION_MAPPER_OK)
			return rc;
	}

	return KN_AX25_ACTION_MAPPER_OK;
}
