/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_timer_replay_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_timer_replay_report.h"

static int test_fail_report(void);
static int test_pass_report(void);

int
main(void)
{
	if (test_pass_report() != 0)
		return 1;
	if (test_fail_report() != 0)
		return 1;

	return 0;
}

static int
test_fail_report(void)
{
	struct kn_ax25_timer_replay_result result;
	char report[KN_AX25_TIMER_REPLAY_REPORT_MAX];

	kn_ax25_timer_replay_result_clear(&result);
	(void)snprintf(result.name, sizeof(result.name), "%s", "fail-case");
	result.pass = 0;
	result.final_state = KN_AX25_CONNECTION_DISCONNECTED;
	result.mismatch_count = 1;
	result.mismatches[0].line = 12;
	(void)snprintf(result.mismatches[0].detail,
	    sizeof(result.mismatches[0].detail), "%s", "state");
	if (kn_ax25_timer_replay_report_format(&result, report,
	    sizeof(report)) != KN_AX25_TIMER_REPLAY_REPORT_OK)
		return 1;
	if (strstr(report, "result=fail") == NULL)
		return 1;

	return strstr(report, "AX25-TIMER-MISMATCH line=12 detail=state") !=
	    NULL ? 0 : 1;
}

static int
test_pass_report(void)
{
	struct kn_ax25_timer_replay_result result;
	char report[KN_AX25_TIMER_REPLAY_REPORT_MAX];

	kn_ax25_timer_replay_result_clear(&result);
	(void)snprintf(result.name, sizeof(result.name), "%s", "pass-case");
	result.pass = 1;
	result.final_state = KN_AX25_CONNECTION_CONNECTED;
	result.retry_count = 1;
	result.t1_running = 1;
	if (kn_ax25_timer_replay_report_format(&result, report,
	    sizeof(report)) != KN_AX25_TIMER_REPLAY_REPORT_OK)
		return 1;
	if (strstr(report, "AX25-TIMER-REPLAY name=pass-case") == NULL)
		return 1;

	return strstr(report, "t1=running") != NULL ? 0 : 1;
}
