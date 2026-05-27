/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_packet_capture.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_packet_capture.h"

static int test_comments_blank(void);
static int test_duplicate_singleton(void);
static int test_invalid_method(void);
static int test_malformed_hex(void);
static int test_missing_frame(void);
static int test_missing_frame_end(void);
static int test_missing_name(void);
static int test_overlong_line(void);
static int test_oversized_frame(void);
static int test_unknown_key(void);
static int test_valid_capture(void);

static const char valid_capture[] =
    "# KiloNode packet capture v1\n"
    "name kiss-ui-ping\n"
    "method kiss\n"
    "direction rx\n"
    "port kiss0\n"
    "frame-begin hex\n"
    "c0 00 86 a2 40 40 40 40 60 9c 60 86 82 98 98 61 03 f0 50 49 4e 47 c0\n"
    "frame-end\n"
    "expect-decode ax25-ui\n"
    "expect-source N0CALL\n"
    "expect-destination CQ\n"
    "expect-pid 0xf0\n"
    "expect-payload-text PING\n";

int
main(void)
{
	if (test_valid_capture() != 0)
		return 1;
	if (test_comments_blank() != 0)
		return 1;
	if (test_missing_name() != 0)
		return 1;
	if (test_invalid_method() != 0)
		return 1;
	if (test_missing_frame() != 0)
		return 1;
	if (test_missing_frame_end() != 0)
		return 1;
	if (test_malformed_hex() != 0)
		return 1;
	if (test_overlong_line() != 0)
		return 1;
	if (test_oversized_frame() != 0)
		return 1;
	if (test_unknown_key() != 0)
		return 1;
	if (test_duplicate_singleton() != 0)
		return 1;

	return 0;
}

static int
test_comments_blank(void)
{
	struct kn_compat_packet_capture capture;

	return kn_compat_packet_capture_parse_text(valid_capture, &capture,
	    NULL) == KN_COMPAT_PACKET_CAPTURE_OK ? 0 : 1;
}

static int
test_duplicate_singleton(void)
{
	const char text[] =
	    "name one\n"
	    "name two\n"
	    "method kiss\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_capture_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_packet_capture_parse_text(text, &capture, &error) !=
	    KN_COMPAT_PACKET_CAPTURE_ERR_DUPLICATE_KEY)
		return 1;

	return error.line == 2 ? 0 : 1;
}

static int
test_invalid_method(void)
{
	const char text[] =
	    "name bad\n"
	    "method bpq\n"
	    "direction rx\n"
	    "port kiss0\n"
	    "frame-begin hex\n"
	    "c0 00 c0\n"
	    "frame-end\n";
	struct kn_compat_packet_capture capture;

	return kn_compat_packet_capture_parse_text(text, &capture, NULL) ==
	    KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_malformed_hex(void)
{
	const char text[] =
	    "name bad\n"
	    "method kiss\n"
	    "direction rx\n"
	    "port kiss0\n"
	    "frame-begin hex\n"
	    "zz\n"
	    "frame-end\n";
	struct kn_compat_packet_capture capture;

	return kn_compat_packet_capture_parse_text(text, &capture, NULL) ==
	    KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_missing_frame(void)
{
	const char text[] =
	    "name bad\n"
	    "method kiss\n"
	    "direction rx\n"
	    "port kiss0\n";
	struct kn_compat_packet_capture capture;

	return kn_compat_packet_capture_parse_text(text, &capture, NULL) ==
	    KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED ? 0 : 1;
}

static int
test_missing_frame_end(void)
{
	const char text[] =
	    "name bad\n"
	    "method kiss\n"
	    "direction rx\n"
	    "port kiss0\n"
	    "frame-begin hex\n"
	    "c0 00 c0\n";
	struct kn_compat_packet_capture capture;

	return kn_compat_packet_capture_parse_text(text, &capture, NULL) ==
	    KN_COMPAT_PACKET_CAPTURE_ERR_PARSE ? 0 : 1;
}

static int
test_missing_name(void)
{
	const char text[] =
	    "method kiss\n"
	    "direction rx\n"
	    "port kiss0\n"
	    "frame-begin hex\n"
	    "c0 00 c0\n"
	    "frame-end\n";
	struct kn_compat_packet_capture capture;

	return kn_compat_packet_capture_parse_text(text, &capture, NULL) ==
	    KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED ? 0 : 1;
}

static int
test_overlong_line(void)
{
	char text[KN_COMPAT_CAPTURE_LINE_MAX + 32];
	struct kn_compat_packet_capture capture;

	memset(text, 'A', sizeof(text));
	text[sizeof(text) - 2] = '\n';
	text[sizeof(text) - 1] = '\0';
	return kn_compat_packet_capture_parse_text(text, &capture, NULL) ==
	    KN_COMPAT_PACKET_CAPTURE_ERR_LINE_TOO_LONG ? 0 : 1;
}

static int
test_oversized_frame(void)
{
	char text[KN_COMPAT_CAPTURE_FRAME_MAX * 4];
	struct kn_compat_packet_capture capture;
	size_t off;
	size_t i;

	off = (size_t)snprintf(text, sizeof(text),
	    "name big\nmethod kiss\ndirection rx\nport kiss0\n"
	    "frame-begin hex\n");
	for (i = 0; i < KN_COMPAT_CAPTURE_FRAME_MAX + 1; i++)
		off += (size_t)snprintf(text + off, sizeof(text) - off,
		    "00\n");
	(void)snprintf(text + off, sizeof(text) - off, "frame-end\n");

	return kn_compat_packet_capture_parse_text(text, &capture, NULL) ==
	    KN_COMPAT_PACKET_CAPTURE_ERR_TOO_LARGE ? 0 : 1;
}

static int
test_unknown_key(void)
{
	const char text[] =
	    "name bad\n"
	    "method kiss\n"
	    "bogus value\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_capture_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_packet_capture_parse_text(text, &capture, &error) !=
	    KN_COMPAT_PACKET_CAPTURE_ERR_UNKNOWN_KEY)
		return 1;

	return error.line == 3 ? 0 : 1;
}

static int
test_valid_capture(void)
{
	struct kn_compat_packet_capture capture;

	if (kn_compat_packet_capture_parse_text(valid_capture, &capture,
	    NULL) != KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (strcmp(capture.name, "kiss-ui-ping") != 0)
		return 1;
	if (capture.method != KN_COMPAT_PACKET_METHOD_KISS)
		return 1;

	return capture.frame_len > 0 ? 0 : 1;
}
