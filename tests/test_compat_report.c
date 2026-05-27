/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_report.h"

static int test_failing_report(void);
static int test_passing_report(void);
static int test_reply_preview_escaped(void);

int
main(void)
{
	if (test_passing_report() != 0)
		return 1;
	if (test_failing_report() != 0)
		return 1;
	if (test_reply_preview_escaped() != 0)
		return 1;

	return 0;
}

static int
test_failing_report(void)
{
	struct kn_compat_replay_result result;
	char report[512];

	kn_compat_replay_result_clear(&result);
	(void)snprintf(result.transcript_name,
	    sizeof(result.transcript_name), "rf-bad");
	result.mode = KN_COMPAT_MODE_RF_UI;
	result.passed = 0;
	result.observed_command = KN_RF_COMMAND_PING;
	result.observed_status = KN_RF_COMMAND_STATUS_OK;
	result.mismatch_count = 1;
	(void)snprintf(result.mismatches[0].text,
	    sizeof(result.mismatches[0].text), "status expected=bad actual=ok");

	if (kn_compat_report_format_text(&result, report, sizeof(report)) !=
	    KN_COMPAT_REPORT_OK)
		return 1;
	if (strstr(report, "result=fail") == NULL)
		return 1;

	return strstr(report, "MISMATCH status expected=bad actual=ok") !=
	    NULL ? 0 : 1;
}

static int
test_passing_report(void)
{
	struct kn_compat_replay_result result;
	char report[512];

	kn_compat_replay_result_clear(&result);
	(void)snprintf(result.transcript_name,
	    sizeof(result.transcript_name), "rf-ping");
	result.mode = KN_COMPAT_MODE_RF_UI;
	result.passed = 1;
	result.observed_command = KN_RF_COMMAND_PING;
	result.observed_status = KN_RF_COMMAND_STATUS_OK;
	result.observed_reply_queued = 1;
	result.observed_tx_frame_id = 1;
	(void)snprintf(result.observed_reply_preview,
	    sizeof(result.observed_reply_preview), "PONG");

	if (kn_compat_report_format_text(&result, report, sizeof(report)) !=
	    KN_COMPAT_REPORT_OK)
		return 1;
	if (strstr(report, "COMPAT transcript=rf-ping") == NULL)
		return 1;
	if (strstr(report, "result=pass") == NULL)
		return 1;

	return strstr(report, "reply=\"PONG\"") != NULL ? 0 : 1;
}

static int
test_reply_preview_escaped(void)
{
	struct kn_compat_replay_result result;
	char report[512];

	kn_compat_replay_result_clear(&result);
	(void)snprintf(result.transcript_name,
	    sizeof(result.transcript_name), "rf-control");
	result.mode = KN_COMPAT_MODE_RF_UI;
	result.passed = 1;
	result.observed_command = KN_RF_COMMAND_PING;
	result.observed_status = KN_RF_COMMAND_STATUS_OK;
	result.observed_reply_preview[0] = 'P';
	result.observed_reply_preview[1] = '\001';
	result.observed_reply_preview[2] = '"';
	result.observed_reply_preview[3] = '\0';

	if (kn_compat_report_format_text(&result, report, sizeof(report)) !=
	    KN_COMPAT_REPORT_OK)
		return 1;

	return strstr(report, "reply=\"P\\x01\\\"\"") != NULL ? 0 : 1;
}
