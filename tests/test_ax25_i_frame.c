/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_i_frame.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_i_frame.h"

static int base_request(struct kn_ax25_i_frame_request *);
static int test_binary_payload(void);
static int test_build_decode_text(void);
static int test_control_roundtrip(void);
static int test_invalid_sequence(void);
static int test_overlarge_payload(void);

int
main(void)
{
	if (test_control_roundtrip() != 0)
		return 1;
	if (test_invalid_sequence() != 0)
		return 1;
	if (test_build_decode_text() != 0)
		return 1;
	if (test_binary_payload() != 0)
		return 1;
	if (test_overlarge_payload() != 0)
		return 1;
	return 0;
}

static int
base_request(struct kn_ax25_i_frame_request *request)
{
	if (request == NULL)
		return 1;
	kn_ax25_i_frame_request_clear(request);
	if (kn_callsign_parse("M6VPN-1", &request->source) != 0 ||
	    kn_callsign_parse("N0CALL", &request->destination) != 0)
		return 1;
	request->max_info_len = 256;
	return 0;
}

static int
test_binary_payload(void)
{
	struct kn_ax25_i_frame_request request;
	struct kn_ax25_i_frame_decoded decoded;
	uint8_t frame[256];
	uint8_t payload[3];
	size_t written;

	payload[0] = 0x00;
	payload[1] = 0xff;
	payload[2] = 0x41;
	if (base_request(&request) != 0)
		return 1;
	request.payload = payload;
	request.payload_len = sizeof(payload);
	if (kn_ax25_i_frame_build(&request, frame, sizeof(frame),
	    &written) != KN_AX25_I_FRAME_OK)
		return 1;
	if (kn_ax25_i_frame_decode_raw(frame, written, &decoded) !=
	    KN_AX25_I_FRAME_OK)
		return 1;
	return decoded.payload_len == sizeof(payload) &&
	    decoded.payload_is_text == 0 ? 0 : 1;
}

static int
test_build_decode_text(void)
{
	struct kn_ax25_i_frame_request request;
	struct kn_ax25_i_frame_decoded decoded;
	const uint8_t payload[] = "hello";
	uint8_t frame[256];
	size_t written;

	if (base_request(&request) != 0)
		return 1;
	request.ns = 2;
	request.nr = 3;
	request.poll_final = 1;
	request.payload = payload;
	request.payload_len = sizeof(payload) - 1;
	if (kn_ax25_i_frame_build(&request, frame, sizeof(frame),
	    &written) != KN_AX25_I_FRAME_OK)
		return 1;
	if (kn_ax25_i_frame_decode_raw(frame, written, &decoded) !=
	    KN_AX25_I_FRAME_OK)
		return 1;
	if (decoded.ns != 2 || decoded.nr != 3 || decoded.poll_final != 1)
		return 1;
	return decoded.payload_len == sizeof(payload) - 1 &&
	    memcmp(decoded.payload, payload, sizeof(payload) - 1) == 0 &&
	    decoded.payload_is_text != 0 ? 0 : 1;
}

static int
test_control_roundtrip(void)
{
	struct kn_ax25_control_info info;
	uint8_t ns;
	uint8_t nr;
	uint8_t control;

	for (ns = 0; ns < 8; ns++) {
		for (nr = 0; nr < 8; nr++) {
			if (kn_ax25_control_encode_i(ns, nr, 1,
			    &control) != KN_AX25_CONTROL_OK)
				return 1;
			kn_ax25_control_decode(control, &info);
			if (info.class != KN_AX25_CONTROL_CLASS_I ||
			    info.ns != ns || info.nr != nr ||
			    info.poll_final != 1)
				return 1;
		}
	}
	return 0;
}

static int
test_invalid_sequence(void)
{
	uint8_t control;

	if (kn_ax25_control_encode_i(8, 0, 0, &control) !=
	    KN_AX25_CONTROL_ERR_INVALID_VALUE)
		return 1;
	return kn_ax25_control_encode_i(0, 8, 0, &control) ==
	    KN_AX25_CONTROL_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_overlarge_payload(void)
{
	struct kn_ax25_i_frame_request request;
	uint8_t frame[256];
	uint8_t payload[4];
	size_t written;

	if (base_request(&request) != 0)
		return 1;
	request.payload = payload;
	request.payload_len = sizeof(payload);
	request.max_info_len = 2;
	return kn_ax25_i_frame_build(&request, frame, sizeof(frame),
	    &written) == KN_AX25_I_FRAME_ERR_TOO_LARGE ? 0 : 1;
}
