/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_kiss_capture.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_kiss_capture.h"

static int decode_text(const char *, struct kn_compat_packet_decode *);
static int test_bench_connected_kinds(void);
static int test_expectations_pass(void);
static int test_kiss_decodes(void);
static int test_malformed_rejected(void);
static int test_mismatch_fails(void);
static int test_payload_hex(void);

static const char kiss_ping[] =
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
    "expect-payload-text PING\n"
    "expect-payload-hex 50 49 4e 47\n"
    "expect-kind UI\n";

int
main(void)
{
	if (test_kiss_decodes() != 0)
		return 1;
	if (test_expectations_pass() != 0)
		return 1;
	if (test_payload_hex() != 0)
		return 1;
	if (test_bench_connected_kinds() != 0)
		return 1;
	if (test_malformed_rejected() != 0)
		return 1;
	if (test_mismatch_fails() != 0)
		return 1;

	return 0;
}

static int
decode_text(const char *text, struct kn_compat_packet_decode *decode)
{
	struct kn_compat_packet_capture capture;

	if (kn_compat_packet_capture_parse_text(text, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;

	return kn_compat_kiss_capture_decode(&capture, decode) ==
	    KN_COMPAT_KISS_CAPTURE_OK ? 0 : 1;
}

static int
test_bench_connected_kinds(void)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_file(
	    "../tests/fixtures/bench/kiss-sabm-node.capture", &capture,
	    NULL) != KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (kn_compat_kiss_capture_decode(&capture, &decode) !=
	    KN_COMPAT_KISS_CAPTURE_OK)
		return 1;
	if (strcmp(decode.kind, "SABM") != 0)
		return 1;
	if (kn_compat_packet_capture_parse_file(
	    "../tests/fixtures/bench/kiss-disc-node.capture", &capture,
	    NULL) != KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (kn_compat_kiss_capture_decode(&capture, &decode) !=
	    KN_COMPAT_KISS_CAPTURE_OK)
		return 1;
	if (strcmp(decode.kind, "DISC") != 0)
		return 1;
	if (kn_compat_packet_capture_parse_file(
	    "../tests/fixtures/bench/kiss-rr-node.capture", &capture,
	    NULL) != KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (kn_compat_kiss_capture_decode(&capture, &decode) !=
	    KN_COMPAT_KISS_CAPTURE_OK)
		return 1;

	return strcmp(decode.kind, "RR") == 0 ? 0 : 1;
}

static int
test_expectations_pass(void)
{
	struct kn_compat_packet_decode decode;

	if (decode_text(kiss_ping, &decode) != 0)
		return 1;

	return decode.passed != 0 && decode.mismatch_count == 0 ? 0 : 1;
}

static int
test_kiss_decodes(void)
{
	struct kn_compat_packet_decode decode;

	if (decode_text(kiss_ping, &decode) != 0)
		return 1;
	if (decode.kiss_command != 0 || decode.kiss_port != 0)
		return 1;
	if (strcmp(decode.source, "N0CALL") != 0)
		return 1;
	if (strcmp(decode.destination, "CQ") != 0)
		return 1;
	if (decode.has_pid == 0 || decode.pid != 0xf0)
		return 1;

	return strcmp(decode.payload_preview, "PING") == 0 ? 0 : 1;
}

static int
test_malformed_rejected(void)
{
	const char text[] =
	    "name bad\n"
	    "method kiss\n"
	    "direction rx\n"
	    "port kiss0\n"
	    "frame-begin hex\n"
	    "c0 00 01 02 c0\n"
	    "frame-end\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_text(text, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;

	return kn_compat_kiss_capture_decode(&capture, &decode) ==
	    KN_COMPAT_KISS_CAPTURE_ERR_DECODE ? 0 : 1;
}

static int
test_mismatch_fails(void)
{
	const char text[] =
	    "name kiss-ui-ping\n"
	    "method kiss\n"
	    "direction rx\n"
	    "port kiss0\n"
	    "frame-begin hex\n"
	    "c0 00 86 a2 40 40 40 40 60 9c 60 86 82 98 98 61 03 f0 50 49 4e 47 c0\n"
	    "frame-end\n"
	    "expect-source M6VPN-1\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_text(text, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (kn_compat_kiss_capture_decode(&capture, &decode) !=
	    KN_COMPAT_KISS_CAPTURE_ERR_MISMATCH)
		return 1;

	return decode.mismatch_count > 0 ? 0 : 1;
}

static int
test_payload_hex(void)
{
	struct kn_compat_packet_decode decode;

	return decode_text(kiss_ping, &decode);
}
