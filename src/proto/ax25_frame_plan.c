/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_frame_plan.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_frame_plan.h"

static enum kn_ax25_frame_plan_error callsign_valid(
	const struct kn_callsign *);
static uint8_t type_is_s_frame(enum kn_ax25_frame_plan_type);

static enum kn_ax25_frame_plan_error
callsign_valid(const struct kn_callsign *callsign)
{
	char text[KN_CALLSIGN_MAX + 4];

	if (kn_callsign_format(callsign, text, sizeof(text)) != 0)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;

	return KN_AX25_FRAME_PLAN_OK;
}

static uint8_t
type_is_s_frame(enum kn_ax25_frame_plan_type type)
{
	return type == KN_AX25_FRAME_PLAN_RR ||
	    type == KN_AX25_FRAME_PLAN_RNR ||
	    type == KN_AX25_FRAME_PLAN_REJ;
}

enum kn_ax25_frame_plan_error
kn_ax25_frame_plan_format(const struct kn_ax25_frame_plan *plan,
	char *buf, size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	int needed;

	if (plan == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&plan->source, source, sizeof(source)) != 0 ||
	    kn_callsign_format(&plan->destination, destination,
	    sizeof(destination)) != 0)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "plan=%llu type=%s from=%s to=%s nr=%u pf=%u action=%s "
	    "needs_kiss=%s needs_fx25=%s",
	    (unsigned long long)plan->id,
	    kn_ax25_frame_plan_type_name(plan->type), source, destination,
	    (unsigned int)plan->nr, (unsigned int)plan->poll_final,
	    kn_ax25_action_intent_name(plan->action_source),
	    plan->needs_kiss != 0 ? "true" : "false",
	    plan->needs_fx25 != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_FRAME_PLAN_ERR_BUFFER;

	return KN_AX25_FRAME_PLAN_OK;
}

void
kn_ax25_frame_plan_list_clear(struct kn_ax25_frame_plan_list *list)
{
	if (list == NULL)
		return;

	memset(list, 0, sizeof(*list));
}

enum kn_ax25_frame_plan_error
kn_ax25_frame_plan_list_append(struct kn_ax25_frame_plan_list *list,
	const struct kn_ax25_frame_plan *plan)
{
	if (list == NULL || plan == NULL)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_ARGUMENT;
	if (list->count >= KN_AX25_FRAME_PLAN_LIST_MAX)
		return KN_AX25_FRAME_PLAN_ERR_FULL;
	if (kn_ax25_frame_plan_validate(plan) != KN_AX25_FRAME_PLAN_OK)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;

	list->plans[list->count] = *plan;
	list->plans[list->count].id = (uint64_t)(list->count + 1U);
	list->count++;
	return KN_AX25_FRAME_PLAN_OK;
}

const char *
kn_ax25_frame_plan_type_name(enum kn_ax25_frame_plan_type type)
{
	switch (type) {
	case KN_AX25_FRAME_PLAN_SABM:
		return "SABM";
	case KN_AX25_FRAME_PLAN_SABME:
		return "SABME";
	case KN_AX25_FRAME_PLAN_UA:
		return "UA";
	case KN_AX25_FRAME_PLAN_DM:
		return "DM";
	case KN_AX25_FRAME_PLAN_DISC:
		return "DISC";
	case KN_AX25_FRAME_PLAN_RR:
		return "RR";
	case KN_AX25_FRAME_PLAN_RNR:
		return "RNR";
	case KN_AX25_FRAME_PLAN_REJ:
		return "REJ";
	case KN_AX25_FRAME_PLAN_FRMR:
		return "FRMR";
	case KN_AX25_FRAME_PLAN_UI:
		return "UI";
	case KN_AX25_FRAME_PLAN_UNKNOWN:
		return "unknown";
	}

	return "unknown";
}

enum kn_ax25_frame_plan_error
kn_ax25_frame_plan_validate(const struct kn_ax25_frame_plan *plan)
{
	size_t i;

	if (plan == NULL)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_ARGUMENT;
	if (callsign_valid(&plan->source) != KN_AX25_FRAME_PLAN_OK ||
	    callsign_valid(&plan->destination) != KN_AX25_FRAME_PLAN_OK)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;
	if (plan->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;
	for (i = 0; i < plan->digipeater_count; i++) {
		if (callsign_valid(&plan->digipeaters[i]) !=
		    KN_AX25_FRAME_PLAN_OK)
			return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;
	}
	if (plan->type >= KN_AX25_FRAME_PLAN_UNKNOWN)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;
	if (plan->poll_final > 1 || plan->needs_kiss > 1 ||
	    plan->needs_fx25 > 1)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;
	if ((type_is_s_frame(plan->type) != 0 && plan->nr > 7) ||
	    plan->ns > 7)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;
	if (plan->payload == NULL && plan->payload_len > 0)
		return KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE;

	return KN_AX25_FRAME_PLAN_OK;
}
