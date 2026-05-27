/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_frame_plan.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_frame_plan.h"

static void base_plan(struct kn_ax25_frame_plan *);
static int test_append_multiple(void);
static int test_append_one(void);
static int test_empty_plan_list(void);
static int test_format_deterministic(void);
static int test_needs_fx25_default_false(void);
static int test_no_queue_dispatch_fields(void);
static int test_plan_list_overflow(void);

int
main(void)
{
	if (test_empty_plan_list() != 0)
		return 1;
	if (test_append_one() != 0)
		return 1;
	if (test_append_multiple() != 0)
		return 1;
	if (test_plan_list_overflow() != 0)
		return 1;
	if (test_format_deterministic() != 0)
		return 1;
	if (test_needs_fx25_default_false() != 0)
		return 1;
	if (test_no_queue_dispatch_fields() != 0)
		return 1;

	return 0;
}

static void
base_plan(struct kn_ax25_frame_plan *plan)
{
	memset(plan, 0, sizeof(*plan));
	(void)kn_callsign_parse("M6VPN-1", &plan->source);
	(void)kn_callsign_parse("N0CALL-1", &plan->destination);
	plan->type = KN_AX25_FRAME_PLAN_UA;
	plan->action_source = KN_AX25_ACTION_SEND_UA;
}

static int
test_append_multiple(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_frame_plan_list list;

	base_plan(&plan);
	kn_ax25_frame_plan_list_clear(&list);
	if (kn_ax25_frame_plan_list_append(&list, &plan) !=
	    KN_AX25_FRAME_PLAN_OK)
		return 1;
	plan.type = KN_AX25_FRAME_PLAN_DM;
	plan.action_source = KN_AX25_ACTION_SEND_DM;
	if (kn_ax25_frame_plan_list_append(&list, &plan) !=
	    KN_AX25_FRAME_PLAN_OK)
		return 1;

	return list.count == 2 && list.plans[1].id == 2 ? 0 : 1;
}

static int
test_append_one(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_frame_plan_list list;

	base_plan(&plan);
	kn_ax25_frame_plan_list_clear(&list);
	if (kn_ax25_frame_plan_list_append(&list, &plan) !=
	    KN_AX25_FRAME_PLAN_OK)
		return 1;

	return list.count == 1 && list.plans[0].id == 1 &&
	    list.plans[0].needs_fx25 == 0 ? 0 : 1;
}

static int
test_empty_plan_list(void)
{
	struct kn_ax25_frame_plan_list list;

	kn_ax25_frame_plan_list_clear(&list);
	return list.count == 0 ? 0 : 1;
}

static int
test_format_deterministic(void)
{
	struct kn_ax25_frame_plan plan;
	char line[256];

	base_plan(&plan);
	plan.id = 3;
	if (kn_ax25_frame_plan_format(&plan, line, sizeof(line)) !=
	    KN_AX25_FRAME_PLAN_OK)
		return 1;

	return strcmp(line, "plan=3 type=UA from=M6VPN-1 to=N0CALL-1 "
	    "nr=0 pf=0 action=send-ua needs_kiss=false "
	    "needs_fx25=false") == 0 ? 0 : 1;
}

static int
test_needs_fx25_default_false(void)
{
	struct kn_ax25_frame_plan plan;

	base_plan(&plan);
	return plan.needs_fx25 == 0 ? 0 : 1;
}

static int
test_no_queue_dispatch_fields(void)
{
	struct kn_ax25_frame_plan plan;

	base_plan(&plan);
	return plan.needs_kiss == 0 && plan.needs_fx25 == 0 ? 0 : 1;
}

static int
test_plan_list_overflow(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_frame_plan_list list;
	size_t i;

	base_plan(&plan);
	kn_ax25_frame_plan_list_clear(&list);
	for (i = 0; i < KN_AX25_FRAME_PLAN_LIST_MAX; i++) {
		if (kn_ax25_frame_plan_list_append(&list, &plan) !=
		    KN_AX25_FRAME_PLAN_OK)
			return 1;
	}

	return kn_ax25_frame_plan_list_append(&list, &plan) ==
	    KN_AX25_FRAME_PLAN_ERR_FULL ? 0 : 1;
}
