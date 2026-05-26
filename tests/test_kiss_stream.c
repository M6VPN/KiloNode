/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_kiss_stream.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/buffer.h"
#include "kilonode/kiss.h"
#include "kilonode/kiss_stream.h"

static int expect_frame(struct kn_kiss_stream_parser *, const uint8_t *,
	size_t, uint8_t, uint8_t);
static int test_complete_frame_one_chunk(void);
static int test_empty_frame_ignored(void);
static int test_escaped_fend_payload(void);
static int test_escaped_fesc_payload(void);
static int test_frame_split_many_chunks(void);
static int test_invalid_escape(void);
static int test_multiple_frames_one_chunk(void);
static int test_non_data_command(void);
static int test_oversized_frame(void);
static int test_port_nibble(void);
static int test_repeated_fend_before_frame(void);
static int test_recovery_after_oversized(void);

int
main(void)
{
	if (test_complete_frame_one_chunk() != 0)
		return 1;
	if (test_frame_split_many_chunks() != 0)
		return 1;
	if (test_multiple_frames_one_chunk() != 0)
		return 1;
	if (test_repeated_fend_before_frame() != 0)
		return 1;
	if (test_empty_frame_ignored() != 0)
		return 1;
	if (test_escaped_fend_payload() != 0)
		return 1;
	if (test_escaped_fesc_payload() != 0)
		return 1;
	if (test_invalid_escape() != 0)
		return 1;
	if (test_oversized_frame() != 0)
		return 1;
	if (test_recovery_after_oversized() != 0)
		return 1;
	if (test_non_data_command() != 0)
		return 1;
	if (test_port_nibble() != 0)
		return 1;

	return 0;
}

static int
expect_frame(struct kn_kiss_stream_parser *parser, const uint8_t *expected,
	size_t expected_len, uint8_t command, uint8_t port)
{
	struct kn_kiss_stream_frame frame;
	struct kn_buffer out;
	int failed;

	if (kn_buffer_init(&out, 0) != 0)
		return 1;

	failed = 0;
	if (kn_kiss_stream_pop_frame(parser, &frame,
	    &out) != KN_KISS_STREAM_OK)
		failed = 1;
	else if (frame.command != command || frame.port != port)
		failed = 1;
	else if (out.len != expected_len ||
	    memcmp(out.data, expected, expected_len) != 0)
		failed = 1;

	kn_buffer_free(&out);
	return failed;
}

static int
test_complete_frame_one_chunk(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x00, 0x01, 0x02,
		KN_KISS_FEND };
	const uint8_t expected[] = { 0x00, 0x01, 0x02 };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (expect_frame(&parser, expected, sizeof(expected), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_empty_frame_ignored(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, KN_KISS_FEND, KN_KISS_FEND };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (kn_kiss_stream_has_frame(&parser) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_escaped_fend_payload(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x00, KN_KISS_FESC,
		KN_KISS_TFEND, KN_KISS_FEND };
	const uint8_t expected[] = { 0x00, KN_KISS_FEND };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (expect_frame(&parser, expected, sizeof(expected), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_escaped_fesc_payload(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x00, KN_KISS_FESC,
		KN_KISS_TFESC, KN_KISS_FEND };
	const uint8_t expected[] = { 0x00, KN_KISS_FESC };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (expect_frame(&parser, expected, sizeof(expected), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_frame_split_many_chunks(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x00, 0x01, 0x02,
		KN_KISS_FEND };
	const uint8_t expected[] = { 0x00, 0x01, 0x02 };
	size_t i;

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;

	for (i = 0; i < sizeof(input); i++) {
		if (kn_kiss_stream_feed(&parser, &input[i],
		    1) != KN_KISS_STREAM_OK)
			return 1;
	}

	if (expect_frame(&parser, expected, sizeof(expected), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_invalid_escape(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t bad[] = { KN_KISS_FEND, 0x00, KN_KISS_FESC, 0x55,
		KN_KISS_FEND };
	const uint8_t good[] = { 0x00, 0x44 };
	const uint8_t recovery[] = { 0x00, 0x44, KN_KISS_FEND };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, bad,
	    sizeof(bad)) != KN_KISS_STREAM_ERR_INVALID_ESCAPE)
		return 1;
	if (kn_kiss_stream_has_frame(&parser) != 0)
		return 1;
	if (kn_kiss_stream_feed(&parser, recovery,
	    sizeof(recovery)) != KN_KISS_STREAM_OK)
		return 1;
	if (expect_frame(&parser, good, sizeof(good), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_multiple_frames_one_chunk(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x00, 0x11, KN_KISS_FEND,
		0x00, 0x22, KN_KISS_FEND };
	const uint8_t first[] = { 0x00, 0x11 };
	const uint8_t second[] = { 0x00, 0x22 };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (expect_frame(&parser, first, sizeof(first), 0, 0) != 0)
		return 1;
	if (expect_frame(&parser, second, sizeof(second), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_non_data_command(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x05, 0x33, KN_KISS_FEND };
	const uint8_t expected[] = { 0x05, 0x33 };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (expect_frame(&parser, expected, sizeof(expected), 5, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_oversized_frame(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x00, 0x01, 0x02,
		KN_KISS_FEND };

	if (kn_kiss_stream_init(&parser, 2) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_ERR_OVERSIZED_FRAME)
		return 1;
	if (kn_kiss_stream_has_frame(&parser) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_port_nibble(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, 0x30, 0x99, KN_KISS_FEND };
	const uint8_t expected[] = { 0x30, 0x99 };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (expect_frame(&parser, expected, sizeof(expected), 0, 3) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_repeated_fend_before_frame(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t input[] = { KN_KISS_FEND, KN_KISS_FEND, KN_KISS_FEND,
		0x00, 0x7a, KN_KISS_FEND };
	const uint8_t expected[] = { 0x00, 0x7a };

	if (kn_kiss_stream_init(&parser, 0) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, input,
	    sizeof(input)) != KN_KISS_STREAM_OK)
		return 1;

	if (expect_frame(&parser, expected, sizeof(expected), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}

static int
test_recovery_after_oversized(void)
{
	struct kn_kiss_stream_parser parser;
	const uint8_t bad[] = { KN_KISS_FEND, 0x00, 0x01, 0x02,
		KN_KISS_FEND };
	const uint8_t good[] = { 0x00, 0x55 };
	const uint8_t recovery[] = { 0x00, 0x55, KN_KISS_FEND };

	if (kn_kiss_stream_init(&parser, 2) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_kiss_stream_feed(&parser, bad,
	    sizeof(bad)) != KN_KISS_STREAM_ERR_OVERSIZED_FRAME)
		return 1;
	if (kn_kiss_stream_feed(&parser, recovery,
	    sizeof(recovery)) != KN_KISS_STREAM_OK)
		return 1;
	if (expect_frame(&parser, good, sizeof(good), 0, 0) != 0)
		return 1;

	kn_kiss_stream_free(&parser);
	return 0;
}
