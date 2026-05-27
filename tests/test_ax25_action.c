/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_action.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_action.h"

static int test_empty_list(void);
static int test_formatting(void);
static int test_no_frame_bytes(void);
static int test_overflow(void);
static int test_protocol_error_name(void);
static int test_push_multiple(void);
static int test_push_one(void);

int
main(void)
{
	if (test_empty_list() != 0)
		return 1;
	if (test_push_one() != 0)
		return 1;
	if (test_push_multiple() != 0)
		return 1;
	if (test_overflow() != 0)
		return 1;
	if (test_formatting() != 0)
		return 1;
	if (test_protocol_error_name() != 0)
		return 1;
	if (test_no_frame_bytes() != 0)
		return 1;

	return 0;
}

static int
test_empty_list(void)
{
	struct kn_ax25_action_list list;
	char out[32];

	kn_ax25_action_list_clear(&list);
	if (list.count != 0)
		return 1;
	if (kn_ax25_action_list_format(&list, out, sizeof(out)) !=
	    KN_AX25_ACTION_OK)
		return 1;

	return strcmp(out, "none") == 0 ? 0 : 1;
}

static int
test_formatting(void)
{
	struct kn_ax25_action_list list;
	char out[128];

	kn_ax25_action_list_clear(&list);
	if (kn_ax25_action_list_append(&list, KN_AX25_ACTION_SEND_UA) !=
	    KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_action_list_append_sequence(&list,
	    KN_AX25_ACTION_SEND_RR, 3) != KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_action_list_format(&list, out, sizeof(out)) !=
	    KN_AX25_ACTION_OK)
		return 1;

	return strcmp(out, "send-ua,send-rr:3") == 0 ? 0 : 1;
}

static int
test_no_frame_bytes(void)
{
	struct kn_ax25_action action;

	memset(&action, 0, sizeof(action));
	action.intent = KN_AX25_ACTION_SEND_DM;

	return sizeof(action) <= sizeof(enum kn_ax25_action_intent) + 8 ?
	    0 : 1;
}

static int
test_overflow(void)
{
	struct kn_ax25_action_list list;
	size_t i;

	kn_ax25_action_list_clear(&list);
	for (i = 0; i < KN_AX25_ACTION_LIST_MAX; i++) {
		if (kn_ax25_action_list_append(&list,
		    KN_AX25_ACTION_START_T1) != KN_AX25_ACTION_OK)
			return 1;
	}

	return kn_ax25_action_list_append(&list, KN_AX25_ACTION_START_T1) ==
	    KN_AX25_ACTION_ERR_FULL ? 0 : 1;
}

static int
test_protocol_error_name(void)
{
	return strcmp(kn_ax25_action_intent_name(
	    KN_AX25_ACTION_PROTOCOL_ERROR), "protocol-error") == 0 ? 0 : 1;
}

static int
test_push_multiple(void)
{
	struct kn_ax25_action_list list;

	kn_ax25_action_list_clear(&list);
	if (kn_ax25_action_list_append(&list, KN_AX25_ACTION_SEND_SABM) !=
	    KN_AX25_ACTION_OK)
		return 1;
	if (kn_ax25_action_list_append(&list, KN_AX25_ACTION_START_T1) !=
	    KN_AX25_ACTION_OK)
		return 1;

	return list.count == 2 ? 0 : 1;
}

static int
test_push_one(void)
{
	struct kn_ax25_action_list list;

	kn_ax25_action_list_clear(&list);
	if (kn_ax25_action_list_append(&list, KN_AX25_ACTION_SEND_SABM) !=
	    KN_AX25_ACTION_OK)
		return 1;

	return list.count == 1 &&
	    list.actions[0].intent == KN_AX25_ACTION_SEND_SABM ? 0 : 1;
}
