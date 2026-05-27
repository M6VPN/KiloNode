/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_capture_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_capture_report.h"
#include "kilonode/compat_kiss_capture.h"

static int test_capture_to_transcript(void);
static int test_failing_report(void);
static int test_passing_report(void);
static int test_unsupported_transcript(void);

int
main(void)
{
	if (test_passing_report() != 0)
		return 1;
	if (test_failing_report() != 0)
		return 1;
	if (test_capture_to_transcript() != 0)
		return 1;
	if (test_unsupported_transcript() != 0)
		return 1;

	return 0;
}

static int
test_capture_to_transcript(void)
{
	const char text[] =
	    "name kiss-ui-ping\n"
	    "method kiss\n"
	    "direction rx\n"
	    "port kiss0\n"
	    "frame-begin hex\n"
	    "c0 00 86 a2 40 40 40 40 60 9c 60 86 82 98 98 61 03 f0 50 49 4e 47 c0\n"
	    "frame-end\n";
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;
	char out[1024];

	if (kn_compat_packet_capture_parse_text(text, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return 1;
	if (kn_compat_kiss_capture_decode(&capture, &decode) !=
	    KN_COMPAT_KISS_CAPTURE_OK)
		return 1;
	if (kn_compat_capture_to_transcript(&capture, &decode, out,
	    sizeof(out)) != KN_COMPAT_CAPTURE_REPORT_OK)
		return 1;
	if (strstr(out, "# source=packet-boundary-capture") == NULL)
		return 1;
	if (strstr(out, "expect-event command=PING status=ok") == NULL)
		return 1;

	return strstr(out, "input PING") != NULL ? 0 : 1;
}

static int
test_failing_report(void)
{
	struct kn_compat_packet_decode decode;
	char report[512];

	kn_compat_packet_decode_clear(&decode);
	(void)snprintf(decode.capture_name, sizeof(decode.capture_name),
	    "bad");
	decode.method = KN_COMPAT_PACKET_METHOD_KISS;
	decode.passed = 0;
	(void)snprintf(decode.source, sizeof(decode.source), "N0CALL");
	(void)snprintf(decode.destination, sizeof(decode.destination), "CQ");
	(void)snprintf(decode.kind, sizeof(decode.kind), "UI");
	decode.mismatch_count = 1;
	(void)snprintf(decode.mismatches[0].text,
	    sizeof(decode.mismatches[0].text), "source");

	if (kn_compat_capture_report_format(&decode, report,
	    sizeof(report)) != KN_COMPAT_CAPTURE_REPORT_OK)
		return 1;
	if (strstr(report, "result=fail") == NULL)
		return 1;

	return strstr(report, "MISMATCH source") != NULL ? 0 : 1;
}

static int
test_passing_report(void)
{
	struct kn_compat_packet_decode decode;
	char report[512];

	kn_compat_packet_decode_clear(&decode);
	(void)snprintf(decode.capture_name, sizeof(decode.capture_name),
	    "kiss-ui-ping");
	decode.method = KN_COMPAT_PACKET_METHOD_KISS;
	decode.passed = 1;
	decode.has_pid = 1;
	decode.pid = 0xf0;
	decode.payload_len = 4;
	(void)snprintf(decode.source, sizeof(decode.source), "N0CALL");
	(void)snprintf(decode.destination, sizeof(decode.destination), "CQ");
	(void)snprintf(decode.kind, sizeof(decode.kind), "UI");
	(void)snprintf(decode.payload_preview, sizeof(decode.payload_preview),
	    "PING");

	if (kn_compat_capture_report_format(&decode, report,
	    sizeof(report)) != KN_COMPAT_CAPTURE_REPORT_OK)
		return 1;
	if (strstr(report, "result=pass") == NULL)
		return 1;

	return strstr(report, "preview=\"PING\"") != NULL ? 0 : 1;
}

static int
test_unsupported_transcript(void)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;
	char out[128];

	kn_compat_packet_capture_clear(&capture);
	kn_compat_packet_decode_clear(&decode);
	(void)snprintf(decode.kind, sizeof(decode.kind), "I");

	return kn_compat_capture_to_transcript(&capture, &decode, out,
	    sizeof(out)) == KN_COMPAT_CAPTURE_REPORT_ERR_UNSUPPORTED ? 0 : 1;
}
