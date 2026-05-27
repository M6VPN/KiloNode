/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_action_mapper.c */

#include <sys/types.h>

#include "kilonode/ax25_action_mapper.h"

static void context(struct kn_ax25_action_mapper_context *);
static int map_one(enum kn_ax25_action_intent,
	enum kn_ax25_frame_plan_type);
static int test_deliver_maps_to_no_frame(void);
static int test_multiple_actions(void);
static int test_overflow_rejected(void);
static int test_protocol_error_maps_to_no_frame(void);
static int test_response_direction(void);
static int test_send_disc(void);
static int test_send_dm(void);
static int test_send_rej(void);
static int test_send_rnr(void);
static int test_send_rr(void);
static int test_send_sabm(void);
static int test_send_sabme(void);
static int test_send_ua(void);
static int test_timer_maps_to_no_frame(void);

int
main(void)
{
	if (test_send_sabm() != 0)
		return 1;
	if (test_send_sabme() != 0)
		return 1;
	if (test_send_ua() != 0)
		return 1;
	if (test_send_dm() != 0)
		return 1;
	if (test_send_disc() != 0)
		return 1;
	if (test_send_rr() != 0)
		return 1;
	if (test_send_rnr() != 0)
		return 1;
	if (test_send_rej() != 0)
		return 1;
	if (test_deliver_maps_to_no_frame() != 0)
		return 1;
	if (test_timer_maps_to_no_frame() != 0)
		return 1;
	if (test_protocol_error_maps_to_no_frame() != 0)
		return 1;
	if (test_multiple_actions() != 0)
		return 1;
	if (test_overflow_rejected() != 0)
		return 1;
	if (test_response_direction() != 0)
		return 1;

	return 0;
}

static void
context(struct kn_ax25_action_mapper_context *ctx)
{
	(void)kn_ax25_action_mapper_context_init(ctx, "M6VPN-1",
	    "N0CALL-1");
	ctx->receive_state = 4;
	ctx->send_state = 2;
}

static int
map_one(enum kn_ax25_action_intent intent, enum kn_ax25_frame_plan_type type)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action action;
	struct kn_ax25_frame_plan_list plans;

	context(&ctx);
	kn_ax25_frame_plan_list_clear(&plans);
	action.intent = intent;
	action.sequence = 5;
	if (kn_ax25_action_mapper_map_action(&ctx, &action, &plans) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return 1;
	if (plans.count != 1 || plans.plans[0].type != type)
		return 1;

	return 0;
}

static int
test_deliver_maps_to_no_frame(void)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action action;
	struct kn_ax25_frame_plan_list plans;

	context(&ctx);
	kn_ax25_frame_plan_list_clear(&plans);
	action.intent = KN_AX25_ACTION_DELIVER_I_PAYLOAD;
	action.sequence = 0;
	if (kn_ax25_action_mapper_map_action(&ctx, &action, &plans) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return 1;

	return plans.count == 0 ? 0 : 1;
}

static int
test_multiple_actions(void)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action_list actions;
	struct kn_ax25_frame_plan_list plans;

	context(&ctx);
	kn_ax25_action_list_clear(&actions);
	kn_ax25_frame_plan_list_clear(&plans);
	(void)kn_ax25_action_list_append(&actions, KN_AX25_ACTION_SEND_UA);
	(void)kn_ax25_action_list_append(&actions, KN_AX25_ACTION_START_T3);
	(void)kn_ax25_action_list_append_sequence(&actions,
	    KN_AX25_ACTION_SEND_RR, 4);
	if (kn_ax25_action_mapper_map_list(&ctx, &actions, &plans) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return 1;

	return plans.count == 2 && plans.plans[0].type ==
	    KN_AX25_FRAME_PLAN_UA && plans.plans[1].type ==
	    KN_AX25_FRAME_PLAN_RR ? 0 : 1;
}

static int
test_overflow_rejected(void)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action action;
	struct kn_ax25_frame_plan_list plans;
	size_t i;

	context(&ctx);
	kn_ax25_frame_plan_list_clear(&plans);
	action.intent = KN_AX25_ACTION_SEND_UA;
	action.sequence = 0;
	for (i = 0; i < KN_AX25_FRAME_PLAN_LIST_MAX; i++) {
		if (kn_ax25_action_mapper_map_action(&ctx, &action,
		    &plans) != KN_AX25_ACTION_MAPPER_OK)
			return 1;
	}

	return kn_ax25_action_mapper_map_action(&ctx, &action, &plans) ==
	    KN_AX25_ACTION_MAPPER_ERR_PLAN_OVERFLOW ? 0 : 1;
}

static int
test_protocol_error_maps_to_no_frame(void)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action action;
	struct kn_ax25_frame_plan_list plans;

	context(&ctx);
	kn_ax25_frame_plan_list_clear(&plans);
	action.intent = KN_AX25_ACTION_PROTOCOL_ERROR;
	action.sequence = 0;
	if (kn_ax25_action_mapper_map_action(&ctx, &action, &plans) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return 1;

	return plans.count == 0 ? 0 : 1;
}

static int
test_response_direction(void)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action action;
	struct kn_ax25_frame_plan_list plans;

	context(&ctx);
	kn_ax25_frame_plan_list_clear(&plans);
	action.intent = KN_AX25_ACTION_SEND_UA;
	action.sequence = 0;
	if (kn_ax25_action_mapper_map_action(&ctx, &action, &plans) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return 1;

	return plans.count == 1 &&
	    plans.plans[0].source.ssid == 1 &&
	    plans.plans[0].destination.ssid == 1 &&
	    plans.plans[0].source.call[0] == 'M' &&
	    plans.plans[0].destination.call[0] == 'N' ? 0 : 1;
}

static int
test_send_disc(void)
{
	return map_one(KN_AX25_ACTION_SEND_DISC, KN_AX25_FRAME_PLAN_DISC);
}

static int
test_send_dm(void)
{
	return map_one(KN_AX25_ACTION_SEND_DM, KN_AX25_FRAME_PLAN_DM);
}

static int
test_send_rej(void)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action action;
	struct kn_ax25_frame_plan_list plans;

	context(&ctx);
	kn_ax25_frame_plan_list_clear(&plans);
	action.intent = KN_AX25_ACTION_SEND_REJ;
	action.sequence = 6;
	if (kn_ax25_action_mapper_map_action(&ctx, &action, &plans) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return 1;

	return plans.count == 1 && plans.plans[0].type ==
	    KN_AX25_FRAME_PLAN_REJ && plans.plans[0].nr == 6 ? 0 : 1;
}

static int
test_send_rnr(void)
{
	return map_one(KN_AX25_ACTION_SEND_RNR, KN_AX25_FRAME_PLAN_RNR);
}

static int
test_send_rr(void)
{
	return map_one(KN_AX25_ACTION_SEND_RR, KN_AX25_FRAME_PLAN_RR);
}

static int
test_send_sabm(void)
{
	return map_one(KN_AX25_ACTION_SEND_SABM, KN_AX25_FRAME_PLAN_SABM);
}

static int
test_send_sabme(void)
{
	return map_one(KN_AX25_ACTION_SEND_SABME, KN_AX25_FRAME_PLAN_SABME);
}

static int
test_send_ua(void)
{
	return map_one(KN_AX25_ACTION_SEND_UA, KN_AX25_FRAME_PLAN_UA);
}

static int
test_timer_maps_to_no_frame(void)
{
	struct kn_ax25_action_mapper_context ctx;
	struct kn_ax25_action action;
	struct kn_ax25_frame_plan_list plans;

	context(&ctx);
	kn_ax25_frame_plan_list_clear(&plans);
	action.intent = KN_AX25_ACTION_START_T1;
	action.sequence = 0;
	if (kn_ax25_action_mapper_map_action(&ctx, &action, &plans) !=
	    KN_AX25_ACTION_MAPPER_OK)
		return 1;

	return plans.count == 0 ? 0 : 1;
}
