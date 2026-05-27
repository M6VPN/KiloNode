/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_connection_diag.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_connection_diag.h"

static void base_plan(struct kn_ax25_frame_plan *);
static int test_format_actions(void);
static int test_format_empty_table(void);
static int test_format_frame_plans(void);
static int test_format_key(void);
static int test_format_one_connection(void);
static int test_format_state(void);
static int test_output_truncation(void);
static int test_unsafe_text_rejected(void);

int
main(void)
{
	if (test_format_empty_table() != 0)
		return 1;
	if (test_format_one_connection() != 0)
		return 1;
	if (test_format_key() != 0)
		return 1;
	if (test_format_state() != 0)
		return 1;
	if (test_format_actions() != 0)
		return 1;
	if (test_format_frame_plans() != 0)
		return 1;
	if (test_output_truncation() != 0)
		return 1;
	if (test_unsafe_text_rejected() != 0)
		return 1;

	return 0;
}

static void
base_plan(struct kn_ax25_frame_plan *plan)
{
	(void)memset(plan, 0, sizeof(*plan));
	(void)kn_callsign_parse("M6VPN-1", &plan->source);
	(void)kn_callsign_parse("N0CALL-1", &plan->destination);
	plan->type = KN_AX25_FRAME_PLAN_UA;
	plan->action_source = KN_AX25_ACTION_SEND_UA;
}

static int
test_format_actions(void)
{
	struct kn_ax25_action_list actions;
	char out[128];

	kn_ax25_action_list_clear(&actions);
	(void)kn_ax25_action_list_append(&actions, KN_AX25_ACTION_SEND_UA);

	if (kn_ax25_connection_diag_format_actions(&actions, out,
	    sizeof(out)) != KN_AX25_CONNECTION_DIAG_OK)
		return 1;

	return strcmp(out, "send-ua") == 0 ? 0 : 1;
}

static int
test_format_empty_table(void)
{
	struct kn_ax25_connection_table table;
	char out[128];

	kn_ax25_connection_table_init(&table);
	if (kn_ax25_connection_diag_format_table(&table, out,
	    sizeof(out)) != KN_AX25_CONNECTION_DIAG_OK)
		return 1;

	return strcmp(out, "AX25 TABLE count=0 max=32") == 0 ? 0 : 1;
}

static int
test_format_frame_plans(void)
{
	struct kn_ax25_frame_plan plan;
	struct kn_ax25_frame_plan_list plans;
	char out[256];

	base_plan(&plan);
	kn_ax25_frame_plan_list_clear(&plans);
	(void)kn_ax25_frame_plan_list_append(&plans, &plan);

	if (kn_ax25_connection_diag_format_frame_plans(&plans, out,
	    sizeof(out)) != KN_AX25_CONNECTION_DIAG_OK)
		return 1;

	return strstr(out, "type=UA") != NULL ? 0 : 1;
}

static int
test_format_key(void)
{
	struct kn_ax25_connection_key key;
	char out[128];

	(void)kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);
	if (kn_ax25_connection_diag_format_key(&key, out, sizeof(out)) !=
	    KN_AX25_CONNECTION_DIAG_OK)
		return 1;

	return strstr(out, "local=M6VPN-1") != NULL ? 0 : 1;
}

static int
test_format_one_connection(void)
{
	struct kn_ax25_connection_record record;
	struct kn_ax25_params params;
	char out[256];

	(void)memset(&record, 0, sizeof(record));
	(void)kn_ax25_connection_key_from_callsigns(&record.key, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);
	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	kn_ax25_connection_init(&record.connection, &params);
	record.connection.state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
	record.last_event_kind = KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST;

	if (kn_ax25_connection_diag_format_record(&record, out,
	    sizeof(out)) != KN_AX25_CONNECTION_DIAG_OK)
		return 1;

	return strstr(out, "state=awaiting-connection") != NULL ? 0 : 1;
}

static int
test_format_state(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_params params;
	char out[192];

	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	kn_ax25_connection_init(&connection, &params);
	connection.state = KN_AX25_CONNECTION_CONNECTED;

	if (kn_ax25_connection_diag_format_state(&connection, out,
	    sizeof(out)) !=
	    KN_AX25_CONNECTION_DIAG_OK)
		return 1;

	return strstr(out, "state=connected") != NULL ? 0 : 1;
}

static int
test_output_truncation(void)
{
	struct kn_ax25_connection_table table;
	char out[4];

	kn_ax25_connection_table_init(&table);

	return kn_ax25_connection_diag_format_table(&table, out,
	    sizeof(out)) == KN_AX25_CONNECTION_DIAG_ERR_BUFFER ? 0 : 1;
}

static int
test_unsafe_text_rejected(void)
{
	struct kn_ax25_connection_key key;
	char out[128];

	kn_ax25_connection_key_clear(&key);
	(void)memcpy(key.port_name, "bad port", 9);

	return kn_ax25_connection_diag_format_key(&key, out,
	    sizeof(out)) != KN_AX25_CONNECTION_DIAG_OK ? 0 : 1;
}
