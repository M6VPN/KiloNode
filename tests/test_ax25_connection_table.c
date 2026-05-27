/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_connection_table.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/ax25_connection_table.h"
#include "kilonode/ax25_control.h"

static void enable_table(struct kn_ax25_connection_table *);
static void fill_frame(struct kn_ax25_frame *, uint8_t);
static int make_key(struct kn_ax25_connection_key *, const char *);
static int make_rx_event(struct kn_ax25_connection_event_record *,
    uint64_t, uint8_t);
static int process_local_connect(struct kn_ax25_connection_table *,
    struct kn_ax25_connection_table_result *);
static int process_rx(struct kn_ax25_connection_table *, uint8_t,
    struct kn_ax25_connection_table_result *);
static int test_connected_rx_i_valid(void);
static int test_connected_rx_rr_updates_sequence(void);
static int test_empty_table(void);
static int test_find_existing(void);
static int test_full_table_rejects_new(void);
static int test_generated_frame_plans_retained(void);
static int test_get_create_first_connection(void);
static int test_inbound_disc_produces_ua_plan(void);
static int test_inbound_sabm_creates_connection(void);
static int test_local_connect_produces_sabm_plan(void);
static int test_no_transport_writes(void);
static int test_remove_connection(void);
static int test_reset_free_safety(void);
static int test_rx_ua_moves_to_connected(void);

int
main(void)
{
	if (test_empty_table() != 0)
		return 1;
	if (test_get_create_first_connection() != 0)
		return 1;
	if (test_find_existing() != 0)
		return 1;
	if (test_remove_connection() != 0)
		return 1;
	if (test_full_table_rejects_new() != 0)
		return 1;
	if (test_inbound_sabm_creates_connection() != 0)
		return 1;
	if (test_inbound_disc_produces_ua_plan() != 0)
		return 1;
	if (test_local_connect_produces_sabm_plan() != 0)
		return 1;
	if (test_rx_ua_moves_to_connected() != 0)
		return 1;
	if (test_connected_rx_rr_updates_sequence() != 0)
		return 1;
	if (test_connected_rx_i_valid() != 0)
		return 1;
	if (test_generated_frame_plans_retained() != 0)
		return 1;
	if (test_no_transport_writes() != 0)
		return 1;
	if (test_reset_free_safety() != 0)
		return 1;

	return 0;
}

static void
enable_table(struct kn_ax25_connection_table *table)
{
	struct kn_ax25_params params;

	kn_ax25_connection_table_init(table);
	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	params.n2_retry_count = 3;
	(void)kn_ax25_connection_table_set_params(table, &params);
}

static void
fill_frame(struct kn_ax25_frame *frame, uint8_t control)
{
	kn_ax25_frame_reset(frame);
	(void)kn_callsign_parse("M6VPN-1", &frame->destination.callsign);
	(void)kn_callsign_parse("N0CALL-1", &frame->source.callsign);
	frame->control = control;
}

static int
make_key(struct kn_ax25_connection_key *key, const char *remote)
{
	return kn_ax25_connection_key_from_callsigns(key, "kiss0",
	    "M6VPN-1", remote, NULL, 0) == KN_AX25_CONNECTION_KEY_OK ?
	    0 : 1;
}

static int
make_rx_event(struct kn_ax25_connection_event_record *event,
    uint64_t timestamp, uint8_t control)
{
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, control);
	(void)kn_callsign_parse("M6VPN-1", &local);

	return kn_ax25_connection_event_from_frame(event, timestamp,
	    "kiss0", &local, &frame) == KN_AX25_CONNECTION_EVENT_OK ?
	    0 : 1;
}

static int
process_local_connect(struct kn_ax25_connection_table *table,
    struct kn_ax25_connection_table_result *result)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_key key;

	if (make_key(&key, "N0CALL-1") != 0)
		return 1;
	if (kn_ax25_connection_event_local_connect(&event, 1, &key) !=
	    KN_AX25_CONNECTION_EVENT_OK)
		return 1;
	if (kn_ax25_connection_table_process(table, &event, result) !=
	    KN_AX25_CONNECTION_TABLE_OK)
		return 1;

	return 0;
}

static int
process_rx(struct kn_ax25_connection_table *table, uint8_t control,
    struct kn_ax25_connection_table_result *result)
{
	struct kn_ax25_connection_event_record event;

	if (make_rx_event(&event, 2, control) != 0)
		return 1;
	if (kn_ax25_connection_table_process(table, &event, result) !=
	    KN_AX25_CONNECTION_TABLE_OK)
		return 1;

	return 0;
}

static int
test_connected_rx_i_valid(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;

	enable_table(&table);
	if (process_rx(&table, 0x2f, &result) != 0)
		return 1;
	if (process_rx(&table, 0x00, &result) != 0)
		return 1;

	return result.state_status == KN_AX25_STATE_OK &&
	    result.actions.count == 2 &&
	    result.plans.count == 1 &&
	    result.plans.plans[0].type == KN_AX25_FRAME_PLAN_RR ? 0 : 1;
}

static int
test_connected_rx_rr_updates_sequence(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;
	uint8_t control;

	enable_table(&table);
	if (process_rx(&table, 0x2f, &result) != 0)
		return 1;
	record = kn_ax25_connection_table_get(&table, 0);
	if (record == NULL)
		return 1;
	record->connection.send_state = 3;
	if (kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_RR, 3, 0,
	    &control) != KN_AX25_CONTROL_OK)
		return 1;
	if (process_rx(&table, control, &result) != 0)
		return 1;

	return record->connection.acknowledge_state == 3 &&
	    record->connection.remote_busy == 0 ? 0 : 1;
}

static int
test_empty_table(void)
{
	struct kn_ax25_connection_table table;

	kn_ax25_connection_table_init(&table);

	return kn_ax25_connection_table_count(&table) == 0 ? 0 : 1;
}

static int
test_find_existing(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_key key;
	size_t index;

	enable_table(&table);
	if (make_key(&key, "N0CALL-1") != 0)
		return 1;
	if (kn_ax25_connection_table_get_or_create(&table, &key, 1,
	    &index, NULL) != KN_AX25_CONNECTION_TABLE_OK)
		return 1;

	return kn_ax25_connection_table_find(&table, &key, &index) ==
	    KN_AX25_CONNECTION_TABLE_OK && index == 0 ? 0 : 1;
}

static int
test_full_table_rejects_new(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_key first;
	struct kn_ax25_connection_key second;
	size_t index;

	enable_table(&table);
	table.max_connections = 1;
	if (make_key(&first, "N0CALL-1") != 0 ||
	    make_key(&second, "OTHER-1") != 0)
		return 1;
	if (kn_ax25_connection_table_get_or_create(&table, &first, 1,
	    &index, NULL) != KN_AX25_CONNECTION_TABLE_OK)
		return 1;

	return kn_ax25_connection_table_get_or_create(&table, &second, 2,
	    &index, NULL) == KN_AX25_CONNECTION_TABLE_ERR_FULL ? 0 : 1;
}

static int
test_generated_frame_plans_retained(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;

	enable_table(&table);
	if (process_rx(&table, 0x2f, &result) != 0)
		return 1;
	record = kn_ax25_connection_table_get(&table, 0);
	if (record == NULL)
		return 1;

	return record->last_plans.count == 1 &&
	    record->last_plans.plans[0].type == KN_AX25_FRAME_PLAN_UA ?
	    0 : 1;
}

static int
test_get_create_first_connection(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_key key;
	size_t index;
	uint8_t created;

	enable_table(&table);
	if (make_key(&key, "N0CALL-1") != 0)
		return 1;
	if (kn_ax25_connection_table_get_or_create(&table, &key, 1,
	    &index, &created) != KN_AX25_CONNECTION_TABLE_OK)
		return 1;

	return index == 0 && created == 1 &&
	    kn_ax25_connection_table_count(&table) == 1 ? 0 : 1;
}

static int
test_inbound_disc_produces_ua_plan(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;

	enable_table(&table);
	if (process_rx(&table, 0x2f, &result) != 0)
		return 1;
	if (process_rx(&table, 0x43, &result) != 0)
		return 1;

	return result.state_status == KN_AX25_STATE_OK &&
	    result.plans.count == 1 &&
	    result.plans.plans[0].type == KN_AX25_FRAME_PLAN_UA ? 0 : 1;
}

static int
test_inbound_sabm_creates_connection(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;

	enable_table(&table);
	if (process_rx(&table, 0x2f, &result) != 0)
		return 1;
	record = kn_ax25_connection_table_get(&table, result.record_index);
	if (record == NULL)
		return 1;

	return result.created == 1 &&
	    record->connection.state == KN_AX25_CONNECTION_CONNECTED &&
	    result.plans.count == 1 &&
	    result.plans.plans[0].type == KN_AX25_FRAME_PLAN_UA ? 0 : 1;
}

static int
test_local_connect_produces_sabm_plan(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;

	enable_table(&table);
	if (process_local_connect(&table, &result) != 0)
		return 1;
	record = kn_ax25_connection_table_get(&table, 0);
	if (record == NULL)
		return 1;

	return record->connection.state ==
	    KN_AX25_CONNECTION_AWAITING_CONNECTION &&
	    result.plans.count == 1 &&
	    result.plans.plans[0].type == KN_AX25_FRAME_PLAN_SABM ? 0 : 1;
}

static int
test_no_transport_writes(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;

	enable_table(&table);
	if (process_local_connect(&table, &result) != 0)
		return 1;
	record = kn_ax25_connection_table_get(&table, 0);
	if (record == NULL)
		return 1;

	return record->connection.tx_output_allowed == 0 &&
	    result.plans.count == 1 ? 0 : 1;
}

static int
test_remove_connection(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_key key;
	size_t index;

	enable_table(&table);
	if (make_key(&key, "N0CALL-1") != 0)
		return 1;
	if (kn_ax25_connection_table_get_or_create(&table, &key, 1,
	    &index, NULL) != KN_AX25_CONNECTION_TABLE_OK)
		return 1;
	if (kn_ax25_connection_table_remove(&table, &key) !=
	    KN_AX25_CONNECTION_TABLE_OK)
		return 1;

	return kn_ax25_connection_table_count(&table) == 0 ? 0 : 1;
}

static int
test_reset_free_safety(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;

	enable_table(&table);
	if (process_rx(&table, 0x2f, &result) != 0)
		return 1;
	kn_ax25_connection_table_reset(&table);
	if (kn_ax25_connection_table_count(&table) != 0)
		return 1;
	kn_ax25_connection_table_free(&table);

	return kn_ax25_connection_table_count(&table) == 0 ? 0 : 1;
}

static int
test_rx_ua_moves_to_connected(void)
{
	struct kn_ax25_connection_table table;
	struct kn_ax25_connection_table_result result;
	struct kn_ax25_connection_record *record;

	enable_table(&table);
	if (process_local_connect(&table, &result) != 0)
		return 1;
	if (process_rx(&table, 0x63, &result) != 0)
		return 1;
	record = kn_ax25_connection_table_get(&table, 0);
	if (record == NULL)
		return 1;

	return record->connection.state == KN_AX25_CONNECTION_CONNECTED &&
	    result.state_status == KN_AX25_STATE_OK ? 0 : 1;
}
