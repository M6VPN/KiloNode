/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_frame.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_prepared_frame.h"
#include "kilonode/callsign.h"

static int make_plan(struct kn_ax25_frame_plan *,
	enum kn_ax25_frame_plan_type, enum kn_ax25_action_intent);
static int test_build_raw_from_plan(void);
static int test_control_plan_kinds(void);
static int test_hex_preview(void);
static int test_invalid_plan_build_failed(void);
static int test_needs_fx25_default_false(void);

int
main(void)
{
	if (test_build_raw_from_plan() != 0)
		return 1;
	if (test_control_plan_kinds() != 0)
		return 1;
	if (test_invalid_plan_build_failed() != 0)
		return 1;
	if (test_hex_preview() != 0)
		return 1;
	if (test_needs_fx25_default_false() != 0)
		return 1;

	return 0;
}

static int
make_plan(struct kn_ax25_frame_plan *plan, enum kn_ax25_frame_plan_type type,
	enum kn_ax25_action_intent action)
{
	if (plan == NULL)
		return 1;

	memset(plan, 0, sizeof(*plan));
	if (kn_callsign_parse("M6VPN-1", &plan->source) != 0 ||
	    kn_callsign_parse("N0CALL", &plan->destination) != 0)
		return 1;
	plan->type = type;
	plan->action_source = action;
	plan->poll_final = 1;
	if (type == KN_AX25_FRAME_PLAN_RR ||
	    type == KN_AX25_FRAME_PLAN_RNR ||
	    type == KN_AX25_FRAME_PLAN_REJ)
		plan->nr = 1;

	return 0;
}

static int
test_build_raw_from_plan(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_prepared_frame frame;
	char formatted[160];

	if (make_plan(&plan, KN_AX25_FRAME_PLAN_UA,
	    KN_AX25_ACTION_SEND_UA) != 0)
		return 1;
	if (kn_ax25_prepared_frame_from_plan(&frame, &plan, 1, "kiss0",
	    1000, 1) != KN_AX25_PREPARED_FRAME_OK)
		return 1;
	if (frame.status != KN_AX25_PREPARED_FRAME_STATUS_PREPARED)
		return 1;
	if (frame.raw_len == 0)
		return 1;
	if (kn_ax25_prepared_frame_format(&frame, formatted,
	    sizeof(formatted)) != KN_AX25_PREPARED_FRAME_OK)
		return 1;

	return strstr(formatted, "kind=UA") != NULL ? 0 : 1;
}

static int
test_control_plan_kinds(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_prepared_frame frame;
	enum kn_ax25_frame_plan_type types[] = {
		KN_AX25_FRAME_PLAN_DM,
		KN_AX25_FRAME_PLAN_DISC,
		KN_AX25_FRAME_PLAN_RR,
		KN_AX25_FRAME_PLAN_RNR,
		KN_AX25_FRAME_PLAN_REJ
	};
	size_t i;

	for (i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
		if (make_plan(&plan, types[i], KN_AX25_ACTION_SEND_DM) != 0)
			return 1;
		if (kn_ax25_prepared_frame_from_plan(&frame, &plan, 1,
		    "kiss0", 1000, 1) != KN_AX25_PREPARED_FRAME_OK)
			return 1;
		if (frame.status != KN_AX25_PREPARED_FRAME_STATUS_PREPARED ||
		    frame.raw_len == 0)
			return 1;
	}

	return 0;
}

static int
test_hex_preview(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_prepared_frame frame;
	char hex[KN_AX25_PREPARED_FRAME_HEX_PREVIEW * 2U + 1U];

	if (make_plan(&plan, KN_AX25_FRAME_PLAN_UA,
	    KN_AX25_ACTION_SEND_UA) != 0)
		return 1;
	if (kn_ax25_prepared_frame_from_plan(&frame, &plan, 1, "kiss0",
	    1000, 1) != KN_AX25_PREPARED_FRAME_OK)
		return 1;
	if (kn_ax25_prepared_frame_hex_preview(&frame, hex, sizeof(hex)) !=
	    KN_AX25_PREPARED_FRAME_OK)
		return 1;

	return strlen(hex) == frame.raw_len * 2U ? 0 : 1;
}

static int
test_invalid_plan_build_failed(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_prepared_frame frame;

	memset(&plan, 0, sizeof(plan));
	if (kn_ax25_prepared_frame_from_plan(&frame, &plan, 1, "kiss0",
	    1000, 1) != KN_AX25_PREPARED_FRAME_OK)
		return 1;

	return frame.status == KN_AX25_PREPARED_FRAME_STATUS_BUILD_FAILED ?
	    0 : 1;
}

static int
test_needs_fx25_default_false(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_prepared_frame frame;

	if (make_plan(&plan, KN_AX25_FRAME_PLAN_UA,
	    KN_AX25_ACTION_SEND_UA) != 0)
		return 1;
	if (kn_ax25_prepared_frame_from_plan(&frame, &plan, 1, "kiss0",
	    1000, 0) != KN_AX25_PREPARED_FRAME_OK)
		return 1;

	return frame.needs_fx25 == 0 && frame.raw_len == 0 ? 0 : 1;
}
