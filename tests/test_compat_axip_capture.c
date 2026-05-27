/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_axip_capture.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_axip_capture.h"

static int test_axip_decodes(void);
static int test_axudp_method(void);
static int test_bench_sabm_decodes(void);
static int test_malformed_ax25(void);
static int test_mismatch(void);

static const char axip_ping[] =
    "name axip-ui-ping\n"
    "method axip\n"
    "direction rx\n"
    "frame-begin hex\n"
    "86 a2 40 40 40 40 60 9c 60 86 82 98 98 61 03 f0 50 49 4e 47\n"
    "frame-end\n"
    "expect-decode ax25-ui\n"
    "expect-source N0CALL\n"
    "expect-destination CQ\n"
    "expect-pid 0xf0\n"
    "expect-payload-text PING\n"
    "expect-kind UI\n";

int
main(void)
{
	if (test_axip_decodes() != 0)
		return 1;
	if (test_axudp_method() != 0)
		return 1;
	if (test_bench_sabm_decodes() != 0)
		return 1;
	if (test_malformed_ax25() != 0)
		return 1;
	if (test_mismatch() != 0)
		return 1;

	return 0;
}

static int
test_bench_sabm_decodes(void)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_file(
	    "../tests/fixtures/bench/ax25-sabm-node.capture", &capture,
	    NULL) != KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (kn_compat_axip_capture_decode(&capture, &decode) !=
	    KN_COMPAT_AXIP_CAPTURE_OK)
		return 1;
	if (strcmp(decode.source, "N0CALL") != 0)
		return 1;
	if (strcmp(decode.destination, "M6VPN-1") != 0)
		return 1;

	return strcmp(decode.kind, "SABM") == 0 ? 0 : 1;
}

static int
test_axip_decodes(void)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_text(axip_ping, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (kn_compat_axip_capture_decode(&capture, &decode) !=
	    KN_COMPAT_AXIP_CAPTURE_OK)
		return 1;
	if (strcmp(decode.source, "N0CALL") != 0)
		return 1;
	if (strcmp(decode.destination, "CQ") != 0)
		return 1;

	return strcmp(decode.payload_preview, "PING") == 0 ? 0 : 1;
}

static int
test_axudp_method(void)
{
	const char text[] =
	    "name axudp-ui-ping\n"
	    "method axudp\n"
	    "direction rx\n"
	    "frame-begin hex\n"
	    "86 a2 40 40 40 40 60 9c 60 86 82 98 98 61 03 f0 50 49 4e 47\n"
	    "frame-end\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_text(text, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;

	return kn_compat_axip_capture_decode(&capture, &decode) ==
	    KN_COMPAT_AXIP_CAPTURE_OK ? 0 : 1;
}

static int
test_malformed_ax25(void)
{
	const char text[] =
	    "name bad\n"
	    "method axip\n"
	    "direction rx\n"
	    "frame-begin hex\n"
	    "01 02 03\n"
	    "frame-end\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_text(text, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;

	return kn_compat_axip_capture_decode(&capture, &decode) ==
	    KN_COMPAT_AXIP_CAPTURE_ERR_DECODE ? 0 : 1;
}

static int
test_mismatch(void)
{
	const char text[] =
	    "name axip-ui-ping\n"
	    "method axip\n"
	    "direction rx\n"
	    "frame-begin hex\n"
	    "86 a2 40 40 40 40 60 9c 60 86 82 98 98 61 03 f0 50 49 4e 47\n"
	    "frame-end\n"
	    "expect-source M6VPN-1\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;

	if (kn_compat_packet_capture_parse_text(text, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;

	return kn_compat_axip_capture_decode(&capture, &decode) ==
	    KN_COMPAT_AXIP_CAPTURE_ERR_MISMATCH ? 0 : 1;
}
