/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_replay.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_replay.h"

static int parse_and_replay(const char *, struct kn_compat_replay_result *);
static int test_help_passes(void);
static int test_info_passes(void);
static int test_malformed_does_not_run(void);
static int test_mismatch_reply_fails(void);
static int test_mismatch_status_fails(void);
static int test_ping_passes(void);
static int test_replay_never_dispatches(void);
static int test_unknown_passes(void);

static const char ping_text[] =
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
	if (test_help_passes() != 0)
		return 1;
	if (test_info_passes() != 0)
		return 1;
	if (test_ping_passes() != 0)
		return 1;
	if (test_unknown_passes() != 0)
		return 1;
	if (test_mismatch_reply_fails() != 0)
		return 1;
	if (test_mismatch_status_fails() != 0)
		return 1;
	if (test_malformed_does_not_run() != 0)
		return 1;
	if (test_replay_never_dispatches() != 0)
		return 1;

	return 0;
}

static int
parse_and_replay(const char *text, struct kn_compat_replay_result *result)
{
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(text, &transcript, &error) !=
	    KN_COMPAT_TRANSCRIPT_OK)
		return 1;

	return kn_compat_replay_transcript(&transcript, result) ==
	    KN_COMPAT_REPLAY_OK ? 0 : 1;
}

static int
test_help_passes(void)
{
	const char text[] =
	    "name rf-help\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input HELP\n"
	    "expect-event command=HELP status=ok\n"
	    "expect-reply contains=KiloNode commands\n"
	    "expect-reply-queued true\n"
	    "expect-no-dispatch true\n";
	struct kn_compat_replay_result result;

	return parse_and_replay(text, &result);
}

static int
test_info_passes(void)
{
	const char text[] =
	    "name rf-info\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input INFO\n"
	    "expect-event command=INFO status=ok\n"
	    "expect-reply contains=KiloNode call=M6VPN-1\n"
	    "expect-reply-queued true\n"
	    "expect-no-dispatch true\n";
	struct kn_compat_replay_result result;

	return parse_and_replay(text, &result);
}

static int
test_malformed_does_not_run(void)
{
	const char text[] =
	    "name malformed\n"
	    "mode rf-ui\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;
	struct kn_compat_replay_result result;

	kn_compat_transcript_clear(&transcript);
	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(text, &transcript, &error) ==
	    KN_COMPAT_TRANSCRIPT_OK)
		return 1;

	return kn_compat_replay_transcript(&transcript, &result) !=
	    KN_COMPAT_REPLAY_OK ? 0 : 1;
}

static int
test_mismatch_reply_fails(void)
{
	const char text[] =
	    "name rf-ping\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=ok\n"
	    "expect-reply contains=BAD\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;
	struct kn_compat_replay_result result;

	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(text, &transcript, &error) !=
	    KN_COMPAT_TRANSCRIPT_OK)
		return 1;

	if (kn_compat_replay_transcript(&transcript, &result) !=
	    KN_COMPAT_REPLAY_ERR_MISMATCH)
		return 1;

	return result.mismatch_count > 0 ? 0 : 1;
}

static int
test_mismatch_status_fails(void)
{
	const char text[] =
	    "name rf-ping\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input PING\n"
	    "expect-event command=PING status=unknown-command\n"
	    "expect-reply contains=PONG\n";
	struct kn_compat_transcript transcript;
	struct kn_compat_transcript_error_info error;
	struct kn_compat_replay_result result;

	memset(&error, 0, sizeof(error));
	if (kn_compat_transcript_parse_text(text, &transcript, &error) !=
	    KN_COMPAT_TRANSCRIPT_OK)
		return 1;

	return kn_compat_replay_transcript(&transcript, &result) ==
	    KN_COMPAT_REPLAY_ERR_MISMATCH ? 0 : 1;
}

static int
test_ping_passes(void)
{
	struct kn_compat_replay_result result;

	return parse_and_replay(ping_text, &result);
}

static int
test_replay_never_dispatches(void)
{
	struct kn_compat_replay_result result;

	if (parse_and_replay(ping_text, &result) != 0)
		return 1;
	if (result.observed_reply_queued == 0 || result.observed_tx_frame_id == 0)
		return 1;

	return result.mismatch_count == 0 ? 0 : 1;
}

static int
test_unknown_passes(void)
{
	const char text[] =
	    "name rf-unknown\n"
	    "mode rf-ui\n"
	    "node M6VPN-1\n"
	    "port kiss0\n"
	    "source N0CALL\n"
	    "destination M6VPN-1\n"
	    "pid 0xf0\n"
	    "input NOPE\n"
	    "expect-event command=UNKNOWN status=unknown-command\n"
	    "expect-reply contains=ERR unknown command\n"
	    "expect-reply-queued true\n"
	    "expect-no-dispatch true\n";
	struct kn_compat_replay_result result;

	return parse_and_replay(text, &result);
}
