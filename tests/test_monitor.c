/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_monitor.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/buffer.h"
#include "kilonode/callsign.h"
#include "kilonode/monitor.h"

static int addr_set(struct kn_ax25_addr *, const char *);
static int encode_ui(struct kn_buffer *, const uint8_t *, size_t, size_t);
static int expect_line(const uint8_t *, size_t, uint8_t, uint8_t,
	const char *);
static int test_binary_payload(void);
static int test_control_characters_payload(void);
static int test_malformed_ax25(void);
static int test_non_data_command(void);
static int test_simple_ui(void);
static int test_ui_multiple_digipeaters(void);
static int test_ui_one_digipeater(void);

int
main(void)
{
	if (test_simple_ui() != 0)
		return 1;
	if (test_ui_one_digipeater() != 0)
		return 1;
	if (test_ui_multiple_digipeaters() != 0)
		return 1;
	if (test_binary_payload() != 0)
		return 1;
	if (test_malformed_ax25() != 0)
		return 1;
	if (test_non_data_command() != 0)
		return 1;
	if (test_control_characters_payload() != 0)
		return 1;

	return 0;
}

static int
addr_set(struct kn_ax25_addr *addr, const char *input)
{
	if (kn_callsign_parse(input, &addr->callsign) != 0)
		return 1;

	addr->repeated = 0;
	return 0;
}

static int
encode_ui(struct kn_buffer *buf, const uint8_t *payload, size_t payload_len,
	size_t digipeater_count)
{
	struct kn_ax25_frame frame;

	kn_ax25_frame_reset(&frame);

	if (addr_set(&frame.destination, "CQ") != 0)
		return 1;
	if (addr_set(&frame.source, "M6VPN-1") != 0)
		return 1;

	if (digipeater_count > 0) {
		if (addr_set(&frame.digipeaters[0], "WIDE1-1") != 0)
			return 1;
	}

	if (digipeater_count > 1) {
		if (addr_set(&frame.digipeaters[1], "WIDE2-1") != 0)
			return 1;
	}

	frame.digipeater_count = digipeater_count;
	frame.pid = KN_AX25_PID_NO_LAYER_3;
	frame.payload = payload;
	frame.payload_len = payload_len;

	return kn_ax25_ui_frame_encode(&frame, buf) == KN_AX25_OK ? 0 : 1;
}

static int
expect_line(const uint8_t *payload, size_t payload_len, uint8_t port,
	uint8_t command, const char *expected)
{
	char line[256];

	if (kn_monitor_format_kiss(line, sizeof(line), port, command, payload,
	    payload_len) != KN_MONITOR_OK)
		return 1;

	return strcmp(line, expected) == 0 ? 0 : 1;
}

static int
test_binary_payload(void)
{
	struct kn_buffer buf;
	const uint8_t payload[] = { 0x00, 0xff, 0x41, 0x0a, 0x7f };
	const char expected[] =
		"PORT 0 UI M6VPN-1 > CQ: binary len 5 hex 00 ff 41 0a 7f";

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (encode_ui(&buf, payload, sizeof(payload), 0) != 0)
		return 1;

	if (expect_line(buf.data, buf.len, 0, 0, expected) != 0)
		return 1;

	kn_buffer_free(&buf);
	return 0;
}

static int
test_control_characters_payload(void)
{
	struct kn_buffer buf;
	const uint8_t payload[] = { 'h', 'i', 0x1b, '[', '0', 'm' };
	const char expected[] =
		"PORT 0 UI M6VPN-1 > CQ: binary len 6 hex 68 69 1b 5b 30 6d";

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (encode_ui(&buf, payload, sizeof(payload), 0) != 0)
		return 1;

	if (expect_line(buf.data, buf.len, 0, 0, expected) != 0)
		return 1;

	kn_buffer_free(&buf);
	return 0;
}

static int
test_malformed_ax25(void)
{
	const uint8_t bad[] = { 0x01, 0x02, 0x03 };
	const char expected[] = "PORT 0 malformed AX.25 len 3 error 2";

	return expect_line(bad, sizeof(bad), 0, 0, expected);
}

static int
test_non_data_command(void)
{
	const uint8_t payload[] = { 0x01, 0x02, 0x03 };
	const char expected[] = "PORT 2 KISS command 5 len 3";

	return expect_line(payload, sizeof(payload), 2, 5, expected);
}

static int
test_simple_ui(void)
{
	struct kn_buffer buf;
	const uint8_t payload[] = "hello";
	const char expected[] = "PORT 0 UI M6VPN-1 > CQ: hello";

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (encode_ui(&buf, payload, sizeof(payload) - 1, 0) != 0)
		return 1;

	if (expect_line(buf.data, buf.len, 0, 0, expected) != 0)
		return 1;

	kn_buffer_free(&buf);
	return 0;
}

static int
test_ui_multiple_digipeaters(void)
{
	struct kn_buffer buf;
	const uint8_t payload[] = "hello";
	const char expected[] =
		"PORT 0 UI M6VPN-1 > CQ via WIDE1-1,WIDE2-1: hello";

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (encode_ui(&buf, payload, sizeof(payload) - 1, 2) != 0)
		return 1;

	if (expect_line(buf.data, buf.len, 0, 0, expected) != 0)
		return 1;

	kn_buffer_free(&buf);
	return 0;
}

static int
test_ui_one_digipeater(void)
{
	struct kn_buffer buf;
	const uint8_t payload[] = "hello";
	const char expected[] = "PORT 0 UI M6VPN-1 > CQ via WIDE1-1: hello";

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (encode_ui(&buf, payload, sizeof(payload) - 1, 1) != 0)
		return 1;

	if (expect_line(buf.data, buf.len, 0, 0, expected) != 0)
		return 1;

	kn_buffer_free(&buf);
	return 0;
}
