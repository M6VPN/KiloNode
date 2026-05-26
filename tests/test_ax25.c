/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/buffer.h"
#include "kilonode/callsign.h"

static int addr_set(struct kn_ax25_addr *, const char *, uint8_t);
static int addr_same(const struct kn_ax25_addr *, const char *, uint8_t,
	uint8_t);
static int frame_base(struct kn_ax25_frame *);
static int test_address_no_final_bit(void);
static int test_address_round_trip(void);
static int test_binary_payload(void);
static int test_destination_source_only(void);
static int test_invalid_ssid(void);
static int test_malformed_callsign(void);
static int test_multiple_digipeaters(void);
static int test_one_digipeater(void);
static int test_round_trip_simple_ui(void);
static int test_too_many_digipeaters(void);
static int test_too_short_frame(void);
static int test_ui_pid_text_payload(void);

int
main(void)
{
	if (test_address_round_trip() != 0)
		return 1;
	if (test_destination_source_only() != 0)
		return 1;
	if (test_one_digipeater() != 0)
		return 1;
	if (test_multiple_digipeaters() != 0)
		return 1;
	if (test_ui_pid_text_payload() != 0)
		return 1;
	if (test_invalid_ssid() != 0)
		return 1;
	if (test_too_short_frame() != 0)
		return 1;
	if (test_address_no_final_bit() != 0)
		return 1;
	if (test_too_many_digipeaters() != 0)
		return 1;
	if (test_binary_payload() != 0)
		return 1;
	if (test_malformed_callsign() != 0)
		return 1;
	if (test_round_trip_simple_ui() != 0)
		return 1;

	return 0;
}

static int
addr_set(struct kn_ax25_addr *addr, const char *input, uint8_t repeated)
{
	if (kn_callsign_parse(input, &addr->callsign) != 0)
		return 1;

	addr->repeated = repeated;
	return 0;
}

static int
addr_same(const struct kn_ax25_addr *addr, const char *call, uint8_t ssid,
	uint8_t repeated)
{
	if (strcmp(addr->callsign.call, call) != 0)
		return 0;

	if (addr->callsign.ssid != ssid)
		return 0;

	if (addr->repeated != repeated)
		return 0;

	return 1;
}

static int
frame_base(struct kn_ax25_frame *frame)
{
	kn_ax25_frame_reset(frame);

	if (addr_set(&frame->destination, "APRS", 0) != 0)
		return 1;

	if (addr_set(&frame->source, "M6VPN-1", 0) != 0)
		return 1;

	frame->pid = KN_AX25_PID_NO_LAYER_3;
	return 0;
}

static int
test_address_no_final_bit(void)
{
	struct kn_ax25_frame decoded;
	struct kn_ax25_addr dest;
	struct kn_ax25_addr source;
	struct kn_buffer buf;
	enum kn_ax25_error rc;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (addr_set(&dest, "APRS", 0) != 0)
		return 1;
	if (addr_set(&source, "M6VPN-1", 0) != 0)
		return 1;

	if (kn_ax25_address_encode(&dest, 0, &buf) != KN_AX25_OK)
		return 1;
	if (kn_ax25_address_encode(&source, 0, &buf) != KN_AX25_OK)
		return 1;

	rc = kn_ax25_frame_decode(buf.data, buf.len, &decoded);
	kn_buffer_free(&buf);

	return rc == KN_AX25_ERR_UNTERMINATED_ADDRESS ? 0 : 1;
}

static int
test_address_round_trip(void)
{
	struct kn_ax25_addr input;
	struct kn_ax25_addr output;
	struct kn_buffer buf;
	uint8_t final;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;

	if (addr_set(&input, "N0CALL-15", 1) != 0)
		return 1;

	if (kn_ax25_address_encode(&input, 1, &buf) != KN_AX25_OK)
		return 1;

	if (kn_ax25_address_decode(buf.data, buf.len, &output,
	    &final) != KN_AX25_OK)
		return 1;

	kn_buffer_free(&buf);

	if (final != 1)
		return 1;

	return addr_same(&output, "N0CALL", 15, 1) ? 0 : 1;
}

static int
test_binary_payload(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;
	const uint8_t payload[] = { 0x00, 0xff, 0xc0, 0xdb, 0x7f };

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (frame_base(&frame) != 0)
		return 1;

	frame.payload = payload;
	frame.payload_len = sizeof(payload);

	if (kn_ax25_ui_frame_encode(&frame, &buf) != KN_AX25_OK)
		return 1;

	if (kn_ax25_frame_decode(buf.data, buf.len, &decoded) != KN_AX25_OK)
		return 1;

	if (decoded.payload_len != sizeof(payload) ||
	    memcmp(decoded.payload, payload, sizeof(payload)) != 0)
		return 1;

	kn_buffer_free(&buf);
	return 0;
}

static int
test_destination_source_only(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (frame_base(&frame) != 0)
		return 1;

	if (kn_ax25_ui_frame_encode(&frame, &buf) != KN_AX25_OK)
		return 1;

	if (kn_ax25_frame_decode(buf.data, buf.len, &decoded) != KN_AX25_OK)
		return 1;

	kn_buffer_free(&buf);

	if (!addr_same(&decoded.destination, "APRS", 0, 0))
		return 1;
	if (!addr_same(&decoded.source, "M6VPN", 1, 0))
		return 1;

	return decoded.digipeater_count == 0 ? 0 : 1;
}

static int
test_invalid_ssid(void)
{
	struct kn_ax25_addr addr;
	struct kn_buffer buf;
	enum kn_ax25_error rc;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;

	memset(&addr, 0, sizeof(addr));
	memcpy(addr.callsign.call, "N0CALL", 6);
	addr.callsign.ssid = 16;

	rc = kn_ax25_address_encode(&addr, 1, &buf);
	kn_buffer_free(&buf);

	return rc == KN_AX25_ERR_INVALID_SSID ? 0 : 1;
}

static int
test_malformed_callsign(void)
{
	struct kn_ax25_addr addr;
	struct kn_buffer buf;
	enum kn_ax25_error rc;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;

	memset(&addr, 0, sizeof(addr));
	memcpy(addr.callsign.call, "BAD!", 4);

	rc = kn_ax25_address_encode(&addr, 1, &buf);
	kn_buffer_free(&buf);

	return rc == KN_AX25_ERR_MALFORMED_ADDRESS ? 0 : 1;
}

static int
test_multiple_digipeaters(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (frame_base(&frame) != 0)
		return 1;
	if (addr_set(&frame.digipeaters[0], "WIDE1-1", 1) != 0)
		return 1;
	if (addr_set(&frame.digipeaters[1], "WIDE2-2", 0) != 0)
		return 1;

	frame.digipeater_count = 2;

	if (kn_ax25_ui_frame_encode(&frame, &buf) != KN_AX25_OK)
		return 1;
	if (kn_ax25_frame_decode(buf.data, buf.len, &decoded) != KN_AX25_OK)
		return 1;

	kn_buffer_free(&buf);

	if (decoded.digipeater_count != 2)
		return 1;
	if (!addr_same(&decoded.digipeaters[0], "WIDE1", 1, 1))
		return 1;
	if (!addr_same(&decoded.digipeaters[1], "WIDE2", 2, 0))
		return 1;

	return 0;
}

static int
test_one_digipeater(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (frame_base(&frame) != 0)
		return 1;
	if (addr_set(&frame.digipeaters[0], "WIDE1-1", 1) != 0)
		return 1;

	frame.digipeater_count = 1;

	if (kn_ax25_ui_frame_encode(&frame, &buf) != KN_AX25_OK)
		return 1;
	if (kn_ax25_frame_decode(buf.data, buf.len, &decoded) != KN_AX25_OK)
		return 1;

	kn_buffer_free(&buf);

	if (decoded.digipeater_count != 1)
		return 1;

	return addr_same(&decoded.digipeaters[0], "WIDE1", 1, 1) ? 0 : 1;
}

static int
test_round_trip_simple_ui(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_frame decoded;
	struct kn_buffer encoded;
	struct kn_buffer encoded_again;
	const uint8_t payload[] = "round trip";

	if (kn_buffer_init(&encoded, 0) != 0)
		return 1;
	if (kn_buffer_init(&encoded_again, 0) != 0)
		return 1;
	if (frame_base(&frame) != 0)
		return 1;

	frame.payload = payload;
	frame.payload_len = sizeof(payload) - 1;

	if (kn_ax25_ui_frame_encode(&frame, &encoded) != KN_AX25_OK)
		return 1;
	if (kn_ax25_frame_decode(encoded.data, encoded.len, &decoded) != KN_AX25_OK)
		return 1;
	if (kn_ax25_ui_frame_encode(&decoded, &encoded_again) != KN_AX25_OK)
		return 1;

	if (encoded.len != encoded_again.len ||
	    memcmp(encoded.data, encoded_again.data, encoded.len) != 0)
		return 1;

	kn_buffer_free(&encoded);
	kn_buffer_free(&encoded_again);
	return 0;
}

static int
test_too_many_digipeaters(void)
{
	struct kn_ax25_frame decoded;
	struct kn_ax25_addr addr;
	struct kn_buffer buf;
	size_t i;
	enum kn_ax25_error rc;

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (addr_set(&addr, "APRS", 0) != 0)
		return 1;
	if (kn_ax25_address_encode(&addr, 0, &buf) != KN_AX25_OK)
		return 1;
	if (addr_set(&addr, "M6VPN-1", 0) != 0)
		return 1;
	if (kn_ax25_address_encode(&addr, 0, &buf) != KN_AX25_OK)
		return 1;

	for (i = 0; i < KN_AX25_MAX_DIGIS + 1; i++) {
		if (addr_set(&addr, "WIDE1-1", 0) != 0)
			return 1;
		if (kn_ax25_address_encode(&addr,
		    (uint8_t)(i == KN_AX25_MAX_DIGIS), &buf) != KN_AX25_OK)
			return 1;
	}

	rc = kn_ax25_frame_decode(buf.data, buf.len, &decoded);
	kn_buffer_free(&buf);

	return rc == KN_AX25_ERR_TOO_MANY_DIGIS ? 0 : 1;
}

static int
test_too_short_frame(void)
{
	struct kn_ax25_frame decoded;
	const uint8_t data[] = { 0x82, 0xa0, 0xa4 };

	return kn_ax25_frame_decode(data, sizeof(data),
	    &decoded) == KN_AX25_ERR_SHORT_FRAME ? 0 : 1;
}

static int
test_ui_pid_text_payload(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;
	const uint8_t payload[] = "hello";

	if (kn_buffer_init(&buf, 0) != 0)
		return 1;
	if (frame_base(&frame) != 0)
		return 1;

	frame.payload = payload;
	frame.payload_len = sizeof(payload) - 1;

	if (kn_ax25_ui_frame_encode(&frame, &buf) != KN_AX25_OK)
		return 1;
	if (kn_ax25_frame_decode(buf.data, buf.len, &decoded) != KN_AX25_OK)
		return 1;

	if (decoded.control != KN_AX25_CONTROL_UI)
		return 1;
	if (decoded.has_pid == 0 || decoded.pid != KN_AX25_PID_NO_LAYER_3)
		return 1;
	if (decoded.payload_len != sizeof(payload) - 1)
		return 1;

	if (memcmp(decoded.payload, payload, sizeof(payload) - 1) != 0)
		return 1;

	kn_buffer_free(&buf);
	return 0;
}
