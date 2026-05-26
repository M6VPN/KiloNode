/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_heard.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/heard.h"

static int addr_set(struct kn_ax25_addr *, const char *);
static int frame_set(struct kn_ax25_frame *, const char *, const char *);
static int test_binary_payload_length(void);
static int test_clear_table(void);
static int test_destination_updates(void);
static int test_different_ssid_separate(void);
static int test_digipeater_path_updates(void);
static int test_empty_table(void);
static int test_eviction_oldest(void);
static int test_first_last_timestamps(void);
static int test_insert_first_station(void);
static int test_invalid_inputs(void);
static int test_same_call_different_ports(void);
static int test_ui_and_pid_stored(void);
static int test_update_existing_station(void);

int
main(void)
{
	if (test_empty_table() != 0)
		return 1;
	if (test_insert_first_station() != 0)
		return 1;
	if (test_update_existing_station() != 0)
		return 1;
	if (test_same_call_different_ports() != 0)
		return 1;
	if (test_different_ssid_separate() != 0)
		return 1;
	if (test_first_last_timestamps() != 0)
		return 1;
	if (test_destination_updates() != 0)
		return 1;
	if (test_digipeater_path_updates() != 0)
		return 1;
	if (test_ui_and_pid_stored() != 0)
		return 1;
	if (test_binary_payload_length() != 0)
		return 1;
	if (test_eviction_oldest() != 0)
		return 1;
	if (test_invalid_inputs() != 0)
		return 1;
	if (test_clear_table() != 0)
		return 1;

	return 0;
}

static int
addr_set(struct kn_ax25_addr *addr, const char *input)
{
	memset(addr, 0, sizeof(*addr));
	return kn_callsign_parse(input, &addr->callsign);
}

static int
frame_set(struct kn_ax25_frame *frame, const char *source, const char *dest)
{
	static const uint8_t payload[] = { 'o', 'k' };

	kn_ax25_frame_reset(frame);
	if (addr_set(&frame->source, source) != 0)
		return 1;
	if (addr_set(&frame->destination, dest) != 0)
		return 1;
	frame->control = KN_AX25_CONTROL_UI;
	frame->pid = KN_AX25_PID_NO_LAYER_3;
	frame->has_pid = 1;
	frame->payload = payload;
	frame->payload_len = sizeof(payload);
	return 0;
}

static int
test_binary_payload_length(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;
	const uint8_t payload[] = { 0x00, 0xff, 0xc0 };

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	frame.payload = payload;
	frame.payload_len = sizeof(payload);
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;

	return table.entries[0].last_payload_len == sizeof(payload) ? 0 : 1;
}

static int
test_clear_table(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;
	kn_heard_clear(&table);

	return table.count == 0 && table.max_entries == 4 ? 0 : 1;
}

static int
test_destination_updates(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;
	if (addr_set(&frame.destination, "APRS") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 2) != KN_HEARD_OK)
		return 1;

	return strcmp(table.entries[0].last_destination.call, "APRS") == 0 ?
	    0 : 1;
}

static int
test_different_ssid_separate(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 2) != KN_HEARD_OK)
		return 1;

	return table.count == 2 ? 0 : 1;
}

static int
test_digipeater_path_updates(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (addr_set(&frame.digipeaters[0], "WIDE1-1") != 0)
		return 1;
	if (addr_set(&frame.digipeaters[1], "WIDE2-1") != 0)
		return 1;
	frame.digipeater_count = 2;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;

	if (table.entries[0].digipeater_count != 2)
		return 1;
	return strcmp(table.entries[0].digipeaters[1].callsign.call,
	    "WIDE2") == 0 ? 0 : 1;
}

static int
test_empty_table(void)
{
	struct kn_heard_table table;

	kn_heard_init(&table, 4);
	if (kn_heard_count(&table) != 0)
		return 1;

	return kn_heard_entries(&table) == table.entries ? 0 : 1;
}

static int
test_eviction_oldest(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 2);
	if (frame_set(&frame, "M6VPN", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 10) != KN_HEARD_OK)
		return 1;
	if (frame_set(&frame, "N0CALL", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 20) != KN_HEARD_OK)
		return 1;
	if (frame_set(&frame, "TEST", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 30) != KN_HEARD_OK)
		return 1;
	if (table.count != 2)
		return 1;
	if (strcmp(table.entries[0].source.call, "TEST") != 0)
		return 1;

	return strcmp(table.entries[1].source.call, "N0CALL") == 0 ? 0 : 1;
}

static int
test_first_last_timestamps(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 100) != KN_HEARD_OK)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 200) != KN_HEARD_OK)
		return 1;
	if (table.entries[0].first_heard != 100)
		return 1;

	return table.entries[0].last_heard == 200 ? 0 : 1;
}

static int
test_insert_first_station(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;
	if (table.count != 1)
		return 1;
	if (strcmp(table.entries[0].source.call, "M6VPN") != 0)
		return 1;

	return table.entries[0].source.ssid == 1 ? 0 : 1;
}

static int
test_invalid_inputs(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(NULL, "kiss0", &frame, 1) !=
	    KN_HEARD_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_heard_update(&table, "", &frame, 1) !=
	    KN_HEARD_ERR_INVALID_ARGUMENT)
		return 1;
	frame.source.callsign.ssid = 99;

	return kn_heard_update(&table, "kiss0", &frame, 1) ==
	    KN_HEARD_ERR_INVALID_ARGUMENT ? 0 : 1;
}

static int
test_same_call_different_ports(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;
	if (kn_heard_update(&table, "kiss1", &frame, 2) != KN_HEARD_OK)
		return 1;

	return table.count == 2 ? 0 : 1;
}

static int
test_ui_and_pid_stored(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;
	if (table.entries[0].last_ui != 1)
		return 1;
	if (table.entries[0].has_pid != 1)
		return 1;

	return table.entries[0].last_pid == KN_AX25_PID_NO_LAYER_3 ? 0 : 1;
}

static int
test_update_existing_station(void)
{
	struct kn_heard_table table;
	struct kn_ax25_frame frame;

	kn_heard_init(&table, 4);
	if (frame_set(&frame, "M6VPN-1", "CQ") != 0)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 1) != KN_HEARD_OK)
		return 1;
	if (kn_heard_update(&table, "kiss0", &frame, 2) != KN_HEARD_OK)
		return 1;
	if (table.count != 1)
		return 1;

	return table.entries[0].frame_count == 2 ? 0 : 1;
}
