/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_transcript.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_transcript.h"

static int parse_ok(const char *, struct kn_compat_transcript *);
static int test_blank_comments(void);
static int test_duplicate_key(void);
static int test_expect_reply_modes(void);
static int test_invalid_callsigns(void);
static int test_invalid_mode(void);
static int test_invalid_pid(void);
static int test_line_number(void);
static int test_missing_required(void);
static int test_overlong_line(void);
static int test_unknown_key(void);
static int test_valid_rf_ui(void);

static const char valid_text[] =
    "# KiloNode compatibility transcript v1\n"
    "name rf-ping\n"
    "mode rf-ui\n"
    "node M6VPN-1\n"
    "port kiss0\n"
    "source N0CALL\n"
    "destination M6VPN-1\n"
    "pid 0xf0\n"
    "input PING\n"
    "expect-event command=PING status=ok\n"
    "expect-reply contains=PONG\n"
    "expect-reply-queued true\n"
    "expect-no-dispatch true\n";

int
main(void)
{
	if (test_valid_rf_ui() != 0)
		return 1;
	if (test_blank_comments() != 0)
		return 1;
	if (test_missing_required() != 0)
		return 1;
	if (test_invalid_mode() != 0)
		return 1;
	if (test_invalid_callsigns() != 0)
		return 1;
	if (test_invalid_pid() != 0)
		return 1;
	if (test_overlong_line() != 0)
		return 1;
	if (test_unknown_key() != 0)
		return 1;
	if (test_duplicate_key() != 0)
		return 1;
	if (test_expect_reply_modes() != 0)
		return 1;
	if (test_line_number() != 0)
		return 1;

	return 0;
}

static int
parse_ok(const char *text, struct kn_compat_transcript *transcript)
{
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	return kn_compat_transcript_parse_text(text, transcript, &error) ==
	    KN_COMPAT_TRANSCRIPT_OK ? 0 : 1;
}

static int
test_blank_comments(void)
{
	const char text[] =
	    "\n"
	    "   # comment\n"
	    "name rf-help\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 240\n"
	    "input HELP\n"
	    "expect-event command=HELP status=ok\n"
	    "expect-reply contains=commands\n";
	struct kn_compat_transcript transcript;

	return parse_ok(text, &transcript);
}

static int
test_duplicate_key(void)
{
	const char text[] =
	    "name one\n"
	    "name two\n"
	    "mode rf-ui\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(text, &transcript, &error) !=
	    KN_COMPAT_TRANSCRIPT_ERR_DUPLICATE_KEY)
		return 1;

	return error.line == 2 ? 0 : 1;
}

static int
test_expect_reply_modes(void)
{
	const char none_text[] =
	    "name rf-none\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply none\n";
	const char exact_text[] =
	    "name rf-exact\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply exact=PONG\n";
	struct kn_compat_transcript transcript;

	if (parse_ok(none_text, &transcript) != 0)
		return 1;
	if (transcript.expect_reply != KN_COMPAT_REPLY_NONE)
		return 1;
	if (parse_ok(exact_text, &transcript) != 0)
		return 1;

	return transcript.expect_reply == KN_COMPAT_REPLY_EXACT &&
	    strcmp(transcript.expect_reply_text, "PONG") == 0 ? 0 : 1;
}

static int
test_invalid_callsigns(void)
{
	const char bad_node[] =
	    "name bad\n"
	    "mode rf-ui\n"
	    "node BAD-CALL\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply contains=PONG\n";
	const char bad_source[] =
	    "name bad\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source BAD-CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply contains=PONG\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(bad_node, &transcript,
	    &error) != KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE)
		return 1;
	memset(&error, 0, sizeof(error));
	return kn_compat_transcript_parse_text(bad_source, &transcript,
	    &error) == KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_mode(void)
{
	const char text[] =
	    "name rf-ping\n"
	    "mode bpq\n"
	    "node M6VPN-1\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	return kn_compat_transcript_parse_text(text, &transcript, &error) ==
	    KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_pid(void)
{
	const char text[] =
	    "name bad\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0x100\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply contains=PONG\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	return kn_compat_transcript_parse_text(text, &transcript, &error) ==
	    KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_line_number(void)
{
	const char text[] =
	    "name rf-ping\n"
	    "mode rf-ui\n"
	    "bad-key value\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(text, &transcript, &error) !=
	    KN_COMPAT_TRANSCRIPT_ERR_UNKNOWN_KEY)
		return 1;

	return error.line == 3 ? 0 : 1;
}

static int
test_missing_required(void)
{
	const char missing_name[] =
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply contains=PONG\n";
	const char missing_mode[] =
	    "name rf-ping\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply contains=PONG\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(missing_name, &transcript,
	    &error) != KN_COMPAT_TRANSCRIPT_ERR_MISSING_REQUIRED)
		return 1;
	memset(&error, 0, sizeof(error));
	return kn_compat_transcript_parse_text(missing_mode, &transcript,
	    &error) == KN_COMPAT_TRANSCRIPT_ERR_MISSING_REQUIRED ? 0 : 1;
}

static int
test_overlong_line(void)
{
	char text[KN_COMPAT_LINE_MAX + 32];
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(text, 'A', sizeof(text));
	text[sizeof(text) - 2] = '\n';
	text[sizeof(text) - 1] = '\0';
	memset(&error, 0, sizeof(error));

	return kn_compat_transcript_parse_text(text, &transcript, &error) ==
	    KN_COMPAT_TRANSCRIPT_ERR_LINE_TOO_LONG ? 0 : 1;
}

static int
test_unknown_key(void)
{
	const char text[] =
	    "name rf-ping\n"
	    "bogus value\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	return kn_compat_transcript_parse_text(text, &transcript, &error) ==
	    KN_COMPAT_TRANSCRIPT_ERR_UNKNOWN_KEY ? 0 : 1;
}

static int
test_valid_rf_ui(void)
{
	struct kn_compat_transcript transcript;
	char report[512];

	if (parse_ok(valid_text, &transcript) != 0)
		return 1;
	if (strcmp(transcript.name, "rf-ping") != 0 ||
	    transcript.mode != KN_COMPAT_MODE_RF_UI ||
	    transcript.pid != 0xf0 ||
	    transcript.expect_command != KN_RF_COMMAND_PING ||
	    transcript.expect_status != KN_RF_COMMAND_STATUS_OK)
		return 1;
	if (transcript.expect_reply != KN_COMPAT_REPLY_CONTAINS ||
	    strcmp(transcript.expect_reply_text, "PONG") != 0)
		return 1;

	return kn_compat_transcript_report(&transcript, report,
	    sizeof(report)) == KN_COMPAT_TRANSCRIPT_OK ? 0 : 1;
}
